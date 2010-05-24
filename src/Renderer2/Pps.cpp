#include "Renderer.hpp"


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Renderer::Pps::init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements( 1 );

	// create the texes
	fai.createEmpty2D( r.width, r.height, GL_RGB, GL_RGB );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create post-processing stage FBO" );

	fbo.unbind();


	// init the shader and it's vars
	sProg.customLoad( "shaders/Pps.glsl" );
	sProg.bind();

	sProg.uniLocs.isFai = sProg.findUniVar( "isFai" )->getLoc();

	if( ssao.enabled )
	{
		ssao.init();
		sProg.uniLocs.ppsSsaoFai = sProg.findUniVar( "ppsSsaoFai" )->getLoc();
	}

	if( hdr.enabled )
	{
		hdr.init();
		sProg.uniLocs.hdrFai = sProg.findUniVar( "ppsHdrFai" )->getLoc();
	}

	/// @ todo enable lscatt
	/*if( R::Pps::Lscatt::enabled )
	{
		R::Pps::Lscatt::init();
		sProg.uniLocs.lscattFai = sProg.findUniVar( "ppsLscattFai" )->getLoc();
	}*/

}


//=====================================================================================================================================
// run                                                                                                                                =
//=====================================================================================================================================
void Renderer::Pps::run()
{
	if( ssao.enabled )
		ssao.run();

	if( hdr.enabled )
		hdr.run();

	fbo.bind();

	// set GL
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	Renderer::setViewport( 0, 0, r.width, r.height );

	// set shader
	sProg.bind();

	sProg.locTexUnit( sProg.uniLocs.isFai, r.is.fai, 0 ); // the IS FAI

	if( hdr.enabled )
	{
		sProg.locTexUnit( sProg.uniLocs.hdrFai, hdr.fai, 1 );
	}

	if( ssao.enabled )
	{
		sProg.locTexUnit( sProg.uniLocs.ppsSsaoFai, ssao.fai, 2 );
	}

	// draw quad
	Renderer::drawQuad( 0 );

	// unbind FBO
	fbo.unbind();
}
