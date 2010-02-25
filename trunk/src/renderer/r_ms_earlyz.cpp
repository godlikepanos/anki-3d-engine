/*
The file contains functions and vars used for the deferred shading/material stage/early z pass.
*/

#include "renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Scene.h"
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

static ShaderProg* shdr_dp, * shdr_dp_grass; // passes for solid objects and grass-like


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	// create FBO
	fbo.Create();
	fbo.bind();

	// inform the we wont write to color buffers
	fbo.SetNumOfColorAttachements(0);

	// attach the texture
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, r::ms::depth_fai.glId, 0 );

	// test if success
	if( !fbo.CheckStatus() )
		FATAL( "Cannot create earlyZ FBO" );

	// unbind
	fbo.unbind();

	// shaders
	shdr_dp = rsrc::shaders.load( "shaders/dp.glsl" );
	shdr_dp_grass = rsrc::shaders.load( "shaders/dp_grass.glsl" );
}


/*
=======================================================================================================================================
RunPass                                                                                                                               =
=======================================================================================================================================
*/
void RunPass( const Camera& cam )
{
	/*// FBO
	fbo.bind();

	// matrix
	glClear( GL_DEPTH_BUFFER_BIT );
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w * r::rendering_quality, r::h * r::rendering_quality );

	// disable color & blend & enable depth test
	glColorMask( false, false, false, false );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );


	// render all meshes
	for( uint i=0; i<Scene::meshes.size(); i++ )
		renderDepth<Mesh>( *Scene::meshes[i], shdr_dp, shdr_dp_grass );

	// render all smodels
	for( uint i=0; i<Scene::models.size(); i++ )
		renderDepth<model_t>( *Scene::models[i], shdr_dp, shdr_dp_grass );

	glColorMask( true, true, true, true );

	// end
	fbo.unbind();*/
}


}}} // end namespaces

#endif
