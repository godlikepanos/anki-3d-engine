#include "Renderer.hpp"
#include "App.h"
#include "Scene.h"
#include "SkelNode.h"


//=====================================================================================================================================
// Constructor                                                                                                                        =
//=====================================================================================================================================
Renderer::Dbg::Dbg( Renderer& r_ ):
	RenderingStage( r_ ),
	showAxisEnabled( false ),
	showLightsEnabled( false ),
	showSkeletonsEnabled( false ),
	showCamerasEnabled( false )
{
}


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Renderer::Dbg::init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// attach the textures
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r.pps.fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create debug FBO" );

	// unbind
	fbo.unbind();

	// shader
	sProg.customLoad( "shaders/Dbg.glsl" );
}


//=====================================================================================================================================
// runStage                                                                                                                           =
//=====================================================================================================================================
void Renderer::Dbg::run()
{
	const Camera& cam = *r.cam;

	fbo.bind();
	sProg.bind();

	// OGL stuff
	r.setProjectionViewMatrices( cam );
	r.setViewport( 0, 0, r.width, r.height );

	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	//R::renderGrid();
	for( uint i=0; i<app->getScene()->nodes.size(); i++ )
	{
		if
		(
			(app->getScene()->nodes[i]->type == SceneNode::NT_LIGHT && showLightsEnabled) ||
			(app->getScene()->nodes[i]->type == SceneNode::NT_CAMERA && showCamerasEnabled) ||
			app->getScene()->nodes[i]->type == SceneNode::NT_PARTICLE_EMITTER
		)
		{
			app->getScene()->nodes[i]->render();
		}
		else if( app->getScene()->nodes[i]->type == SceneNode::NT_SKELETON && showSkeletonsEnabled )
		{
			SkelNode* skel_node = static_cast<SkelNode*>( app->getScene()->nodes[i] );
			glDisable( GL_DEPTH_TEST );
			skel_node->render();
			glEnable( GL_DEPTH_TEST );
		}
	}
}

