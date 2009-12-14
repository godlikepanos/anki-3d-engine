/*
The file contains functions and vars used for the deferred shading/material stage/early z pass.
*/

#include "renderer.h"
#include "resource.h"
#include "texture.h"
#include "scene.h"
#include "r_private.h"
#include "fbo.h"

#if defined(_EARLY_Z_)

namespace r {
namespace ms {
namespace earlyz {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static fbo_t fbo;

static shader_prog_t* shdr_dp, * shdr_dp_grass; // passes for solid objects and grass-like


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform the we wont write to color buffers
	fbo.SetNumOfColorAttachements(0);

	// attach the texture
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, r::ms::depth_fai.gl_id, 0 );

	// test if success
	if( !fbo.CheckStatus() )
		FATAL( "Cannot create earlyZ FBO" );

	// unbind
	fbo.Unbind();

	// shaders
	shdr_dp = rsrc::shaders.Load( "shaders/dp.shdr" );
	shdr_dp_grass = rsrc::shaders.Load( "shaders/dp_grass.shdr" );
}


/*
=======================================================================================================================================
RunPass                                                                                                                               =
=======================================================================================================================================
*/
void RunPass( const camera_t& cam )
{
	// FBO
	fbo.Bind();

	// matrix
	glClear( GL_DEPTH_BUFFER_BIT );
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w * r::rendering_quality, r::h * r::rendering_quality );

	// disable color & blend & enable depth test
	glColorMask( false, false, false, false );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );


	// render all meshes
	for( uint i=0; i<scene::meshes.size(); i++ )
		RenderDepth<mesh_t>( *scene::meshes[i], shdr_dp, shdr_dp_grass );

	// render all smodels
	for( uint i=0; i<scene::models.size(); i++ )
		RenderDepth<model_t>( *scene::models[i], shdr_dp, shdr_dp_grass );

	glColorMask( true, true, true, true );

	// end
	fbo.Unbind();
}


}}} // end namespaces

#endif
