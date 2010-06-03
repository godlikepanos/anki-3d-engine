#include "Renderer.h"


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Renderer::Ms::EarlyZ::init()
{
	if( !enabled ) return;

	// create FBO
	fbo.create();
	fbo.bind();

	fbo.setNumOfColorAttachements( 0 );

	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0 );

	if( !fbo.isGood() )
		FATAL( "Cannot create shadowmapping FBO" );

	fbo.unbind();
}
