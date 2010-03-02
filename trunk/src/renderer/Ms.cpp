/**
 * The file contains functions and vars used for the deferred shading material stage.
 */

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Mesh.h"
#include "Fbo.h"
#include "Material.h"
#include "MeshNode.h"


namespace R {
namespace Ms {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static Fbo fbo;

Texture normalFai, diffuseFai, specularFai, depthFai;



//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(3);

	// create the FAIs
	const int internal_format = GL_RGBA16F_ARB;
	if( !normalFai.createEmpty2D( R::w, R::h, internal_format, GL_RGBA ) ||
	    !diffuseFai.createEmpty2D( R::w, R::h, internal_format, GL_RGBA ) ||
	    !specularFai.createEmpty2D( R::w, R::h, internal_format, GL_RGBA ) ||
	    !depthFai.createEmpty2D( R::w, R::h, GL_DEPTH24_STENCIL8_EXT, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT ) )
	{
		FATAL( "See prev error" );
	}

	
	// you could use the above for SSAO but the difference is very little.
	//depthFai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//depthFai.texParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	// attach the buffers to the FBO
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, normalFai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, diffuseFai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, specularFai.getGlId(), 0 );

	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depthFai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, depthFai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading material pass FBO" );

	// unbind
	fbo.unbind();

#if defined( _EARLY_Z_ )
	R::Ms::earlyz::init();
#endif
}


//=====================================================================================================================================
// runStage                                                                                                                           =
//=====================================================================================================================================
void runStage( const Camera& cam )
{
	#if defined( _EARLY_Z_ )
		// run the early z pass
		R::Ms::earlyz::runPass( cam );
	#endif

	fbo.bind();

	#if !defined( _EARLY_Z_ )
		glClear( GL_DEPTH_BUFFER_BIT );
	#endif
	R::setProjectionViewMatrices( cam );
	R::setViewport( 0, 0, R::w, R::h );

	//glEnable( GL_DEPTH_TEST );
	Scene::skybox.Render( cam.getViewMatrix().getRotationPart() );
	//glDepthFunc( GL_LEQUAL );

	#if defined( _EARLY_Z_ )
		glDepthMask( false );
		glDepthFunc( GL_EQUAL );
	#endif

	// render the meshes
	for( uint i=0; i<Scene::meshNodes.size(); i++ )
	{
		MeshNode* mesh_node = Scene::meshNodes[i];
		DEBUG_ERR( mesh_node->material == NULL );
		if( mesh_node->material->blends || mesh_node->material->refracts ) continue;
		mesh_node->material->setup();
		mesh_node->render();
	}

	glPolygonMode( GL_FRONT, GL_FILL ); // the rendering above fucks the polygon mode


	#if defined( _EARLY_Z_ )
		glDepthMask( true );
		glDepthFunc( GL_LESS );
	#endif

	fbo.unbind();
}


}} // end namespaces
