#include "Renderer.h"


//======================================================================================================================
// initFbos                                                                                                            =
//======================================================================================================================
void Renderer::Pps::Hdr::initFbos( Fbo& fbo, Texture& fai, int internalFormat )
{
	int width = renderingQuality * r.width;
	int height = renderingQuality * r.height;

	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( width, height, internalFormat, GL_RGB, GL_FLOAT, false );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//fai_.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage HDR passes FBO" );

	// unbind
	fbo.unbind();
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::Pps::Hdr::init()
{
	//int width = renderingQuality * r.width;
	int height = renderingQuality * r.height;

	initFbos( pass0Fbo, pass0Fai, GL_RGB );
	initFbos( pass1Fbo, pass1Fai, GL_RGB );
	initFbos( pass2Fbo, fai, GL_RGB );

	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// init shaders
	const char* shaderFname = "shaders/PpsHdr.glsl";
	string pps;

	pps = "#define _PPS_HDR_PASS_0_\n#define IS_FAI_WIDTH " + Util::floatToStr(r.width) + "\n";
	if( !pass0SProg.customLoad( shaderFname, pps.c_str() ) )
		FATAL( "See prev error" );
	pass0SProg.uniVars.fai = pass0SProg.findUniVar("fai");

	pps = "#define _PPS_HDR_PASS_1_\n#define PASS0_HEIGHT " + Util::floatToStr(height) + "\n";
	if( !pass1SProg.customLoad( shaderFname, pps.c_str() ) )
		FATAL( "See prev error" );
	pass1SProg.uniVars.fai = pass1SProg.findUniVar("fai");

	if( !pass2SProg.customLoad( shaderFname, "#define _PPS_HDR_PASS_2_\n" ) )
		FATAL( "See prev error" );
	pass2SProg.uniVars.fai = pass2SProg.findUniVar("fai");
}


//======================================================================================================================
// runPass                                                                                                             =
//======================================================================================================================
void Renderer::Pps::Hdr::run()
{
	int w = renderingQuality * r.width;
	int h = renderingQuality * r.height;
	Renderer::setViewport( 0, 0, w, h );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	// pass 0
	pass0Fbo.bind();
	pass0SProg.bind();
	pass0SProg.uniVars.fai->setTexture( r.is.fai, 0 );
	Renderer::drawQuad( 0 );

	// pass 1
	pass1Fbo.bind();
	pass1SProg.bind();
	pass1SProg.uniVars.fai->setTexture( pass0Fai, 0 );
	Renderer::drawQuad( 0 );

	// pass 2
	pass2Fbo.bind();
	pass2SProg.bind();
	pass2SProg.uniVars.fai->setTexture( pass1Fai, 0 );
	Renderer::drawQuad( 0 );

	// end
	Fbo::unbind();
}
