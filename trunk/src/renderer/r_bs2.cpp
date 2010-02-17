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
#include "mesh_node.h"
#include "material.h"


namespace r {
namespace bs {

//=====================================================================================================================================
// VARS                                                                                                                               =
//=====================================================================================================================================
static fbo_t intermid_fbo, fbo;

static texture_t fai; ///< RGB for color and A for mask (0 doesnt pass, 1 pass)
static shader_prog_t* shader_prog;


//=====================================================================================================================================
// Init2                                                                                                                              =
//=====================================================================================================================================
void Init2()
{
	//** 1st FBO **
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform FBO about the color buffers
	fbo.SetNumOfColorAttachements(1);

	// attach the texes
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r::pps::fai.GetGLID(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, r::ms::depth_fai.GetGLID(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	fbo.Unbind();


	//** 2nd FBO **
	intermid_fbo.Create();
	intermid_fbo.Bind();

	// texture
	intermid_fbo.SetNumOfColorAttachements(1);
	fai.CreateEmpty2D( r::w, r::h, GL_RGBA8, GL_RGBA );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.GetGLID(), 0 );

	// attach the texes
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, r::ms::depth_fai.GetGLID(), 0 );

	// test if success
	if( !intermid_fbo.IsGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	intermid_fbo.Unbind();

	shader_prog = rsrc::shaders.Load( "shaders/bs_refract.glsl" );
}


//=====================================================================================================================================
// RunStage2                                                                                                                          =
//=====================================================================================================================================
void RunStage2( const camera_t& cam )
{
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w, r::h );


	glDepthMask( false );


	// render the meshes
	for( uint i=0; i<scene::mesh_nodes.size(); i++ )
	{
		mesh_node_t* mesh_node = scene::mesh_nodes[i];
		if( mesh_node->material->refracts )
		{
			// write to the rFbo
			intermid_fbo.Bind();
			glEnable( GL_DEPTH_TEST );
			glClear( GL_COLOR_BUFFER_BIT );
			mesh_node->material->Setup();
			mesh_node->Render();

			fbo.Bind();
			glDisable( GL_DEPTH_TEST );
			shader_prog->Bind();
			shader_prog->LocTexUnit( shader_prog->GetUniformLocation(0), fai, 0 );
			r::DrawQuad( shader_prog->GetAttributeLocation(0) );
		}
	}


	// restore a few things
	glDepthMask( true );
	fbo_t::Unbind();
}


}} // end namespaces
