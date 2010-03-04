/**
 * The file contains functions and vars used for the deferred shading blending stage stage.
 * The blending stage comes after the illumination stage. All the objects that are transculent will be drawn here.
 */

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Mesh.h"
#include "Resource.h"
#include "Fbo.h"
#include "MeshNode.h"
#include "Material.h"
#include "App.h"


namespace R {
namespace Bs {

//=====================================================================================================================================
// VARS                                                                                                                               =
//=====================================================================================================================================
static Fbo fbo; ///< blending models FBO


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform FBO about the color buffers
	fbo.setNumOfColorAttachements(1);

	// attach the texes
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, R::Is::fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, R::Ms::depthFai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	fbo.unbind();
}


//=====================================================================================================================================
// runStage                                                                                                                           =
//=====================================================================================================================================
void runStage( const Camera& cam )
{
	// OGL stuff
	R::setProjectionViewMatrices( cam );
	R::setViewport( 0, 0, R::w, R::h );

	glEnable( GL_DEPTH_TEST );
	glDepthMask( false );



	// render the meshes
	for( uint i=0; i<app->scene->meshNodes.size(); i++ )
	{
		MeshNode* meshNode = app->scene->meshNodes[i];
		if( meshNode->material->blends && !meshNode->material->blends )
		{
			fbo.bind();
			meshNode->material->setup();
			meshNode->render();
		}

	}


	// restore a few things
	glDepthMask( true );
	Fbo::unbind();
}


}} // end namespaces
