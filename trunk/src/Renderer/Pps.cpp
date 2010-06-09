#include "Renderer.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::Pps::init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements( 1 );

	// create the texes
	fai.createEmpty2D( r.width, r.height, GL_RGB, GL_RGB, GL_FLOAT, false );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create post-processing stage FBO" );

	fbo.unbind();


	// init the shader and it's vars
	string pps = "";
	if( ssao.enabled )
	{
		pps += "#define _SSAO_\n";
	}

	if( hdr.enabled )
	{
		pps += "#define _HDR_\n";
	}

	sProg.customLoad( "shaders/Pps.glsl", pps.c_str() );
	sProg.bind();

	sProg.uniVars.isFai = sProg.findUniVar( "isFai" );

	if( ssao.enabled )
	{
		ssao.init();
		sProg.uniVars.ppsSsaoFai = sProg.findUniVar( "ppsSsaoFai" );
	}

	if( hdr.enabled )
	{
		hdr.init();
		sProg.uniVars.hdrFai = sProg.findUniVar( "ppsHdrFai" );
	}

	/// @ todo enable lscatt
	/*if( R::Pps::Lscatt::enabled )
	{
		R::Pps::Lscatt::init();
		sProg.uniVars.lscattFai = sProg.findUniVar( "ppsLscattFai" )->getLoc();
	}*/

}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
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
	sProg.uniVars.isFai->setTexture( r.is.fai, 0 );

	if( hdr.enabled )
	{
		sProg.uniVars.hdrFai->setTexture( hdr.fai, 1 );
	}

	if( ssao.enabled )
	{
		sProg.uniVars.ppsSsaoFai->setTexture( ssao.fai, 2 );
	}

	// draw quad
	Renderer::drawQuad( 0 );

	// unbind FBO
	fbo.unbind();
}
