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

static uint stencil_rb; ///< ToDo

static texture_t r_fai;


/*
=======================================================================================================================================
InitBFBO                                                                                                                              =
=======================================================================================================================================
*/
static void InitBFBO()
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
	if( !b_fbo.CheckStatus() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	b_fbo.Unbind();
}


/*
=======================================================================================================================================
InitRFBO                                                                                                                              =
=======================================================================================================================================
*/
static void InitRFBO()
{
	// create FBO
	r_fbo.Create();
	r_fbo.Bind();

	// init the stencil render buffer
	glGenRenderbuffers( 1, &stencil_rb );
	glBindRenderbuffer( GL_RENDERBUFFER, stencil_rb );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_STENCIL_INDEX, r::w * r::rendering_quality, r::h * r::rendering_quality );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_rb );


	// inform FBO about the color buffers
	r_fbo.SetNumOfColorAttachements(0);

	// texture
	r_fai.CreateEmpty( r::w * r::rendering_quality, r::h * r::rendering_quality, GL_RGB, GL_RGB );

	// attach the texes
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_fai.GetGLID(), 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, r::ms::depth_fai.GetGLID(), 0 );

	// test if success
	if( !r_fbo.CheckStatus() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	r_fbo.Unbind();
}


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void Init()
{
	InitBFBO();
	//InitRFBO();
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

	b_fbo.Bind();

	// render the meshes
	for( uint i=0; i<scene::meshes.size(); i++ )
	{
		Render< mesh_t, true >( scene::meshes[i] );
	}

	// render the smodels
	/*for( uint i=0; i<scene::models.size(); i++ )
		Render<model_t, true>( scene::models[i] );*/


	// restore a few things
	glDepthMask( true );
	fbo_t::Unbind();
}


}} // end namespaces
