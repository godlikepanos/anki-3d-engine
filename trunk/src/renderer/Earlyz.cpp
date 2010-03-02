/*
The file contains functions and vars used for the deferred shading/material stage/early z pass.
*/

#include "Renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Scene.h"
#include "Fbo.h"

#if defined(_EARLY_Z_)

namespace R {
namespace ms {
namespace earlyz {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static Fbo fbo;

static ShaderProg* shdr_dp, * shdr_dp_grass; // passes for solid objects and grass-like


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform the we wont write to color buffers
	fbo.setNumOfColorAttachements(0);

	// attach the texture
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, R::Ms::depthFai.glId, 0 );

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
runPass                                                                                                                               =
=======================================================================================================================================
*/
void runPass( const Camera& cam )
{
	/*// FBO
	fbo.bind();

	// matrix
	glClear( GL_DEPTH_BUFFER_BIT );
	R::setProjectionViewMatrices( cam );
	R::setViewport( 0, 0, R::w * R::renderingQuality, R::h * R::renderingQuality );

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
