/**
 * The file contains functions and vars used for the deferred shading blending stage stage.
 * The blending stage comes after the illumination stage. All the objects that are transculent will be drawn here.
 */

#include "renderer.h"
#include "camera.h"
#include "scene.h"
#include "mesh.h"
#include "r_private.h"
#include "resource.h"
#include "fbo.h"

namespace r {
namespace bs {

/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static fbo_t b_fbo; ///< blending models FBO
static fbo_t r_fbo; ///< refracting models FBO

static texture_t r_fai; ///< RGB for color and A for mask (0 doesnt pass, 1 pass)

static shader_prog_t* r2b_shdr;


/*
=======================================================================================================================================
InitBFBO                                                                                                                              =
=======================================================================================================================================
*/
static void InitB()
{
	// create FBO
	b_fbo.Create();
	b_fbo.Bind();

	// inform FBO about the color buffers
	b_fbo.SetNumOfColorAttachements(1);

	// attach the texes
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r::is::fai.GetGLID(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, r::ms::depth_fai.GetGLID(), 0 );

	// test if success
	if( !b_fbo.IsGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	b_fbo.Unbind();
}


/*
=======================================================================================================================================
InitRFBO                                                                                                                              =
=======================================================================================================================================
*/
static void InitR()
{
	// create FBO
	r_fbo.Create();
	r_fbo.Bind();

	// inform FBO about the color buffers
	r_fbo.SetNumOfColorAttachements(1);

	// texture
	r_fai.CreateEmpty( r::w * r::rendering_quality, r::h * r::rendering_quality, GL_RGBA, GL_RGBA );

	// attach the texes
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_fai.GetGLID(), 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, r::ms::depth_fai.GetGLID(), 0 );

	// test if success
	if( !r_fbo.IsGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	r_fbo.Unbind();

	//r2b_shdr =
}


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void Init()
{
	InitB();
	InitR();
}


//=====================================================================================================================================
// RunStage                                                                                                                           =
//=====================================================================================================================================
void RunStage( const camera_t& cam )
{
	// OGL stuff
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w*r::rendering_quality, r::h*r::rendering_quality );

	glEnable( GL_DEPTH_TEST );
	glDepthMask( false );



	// render the meshes
	for( uint i=0; i<scene::meshes.size(); i++ )
	{
		mesh_node_t* mesh = scene::meshes[i];
		if( mesh->material->blends )
		{
			b_fbo.Bind();
			mesh->material->Setup();
			mesh->Render();
		}
		else if( mesh->material->refracts )
		{
			// write to the rFbo
			r_fbo.Bind();
			glClear( GL_COLOR_BUFFER_BIT );
			mesh->material->Setup();
			mesh->Render();

			b_fbo.Bind();
		}
	}

	// render the smodels
	/*for( uint i=0; i<scene::models.size(); i++ )
		Render<model_t, true>( scene::models[i] );*/


	// restore a few things
	glDepthMask( true );
	fbo_t::Unbind();
}


}} // end namespaces
