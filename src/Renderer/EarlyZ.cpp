#include "Renderer.h"
#include "App.h"
#include "MeshNode.h"
#include "Scene.h"


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Renderer::Ms::EarlyZ::init()
{
	//
	// init FBO
	//
	fbo.create();
	fbo.bind();

	fbo.setNumOfColorAttachements( 0 );

	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0 );

	if( !fbo.isGood() )
		FATAL( "Cannot create shadowmapping FBO" );

	fbo.unbind();
}


//=====================================================================================================================================
// run                                                                                                                                =
//=====================================================================================================================================
void Renderer::Ms::EarlyZ::run()
{
	fbo.bind();

	Renderer::setViewport( 0, 0, r.width, r.height );

	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	for( Vec<MeshNode*>::iterator it=app->getScene()->meshNodes.begin(); it!=app->getScene()->meshNodes.end(); it++ )
	{
		MeshNode* meshNode = (*it);
		if( meshNode->material->blends || meshNode->material->refracts ) continue;

		DEBUG_ERR( meshNode->material->dpMtl == NULL );

		r.setupMaterial( *meshNode->material->dpMtl, *meshNode, *r.cam );
		meshNode->render();
	}

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
}
