#include "Renderer.hpp"


//=====================================================================================================================================
// initFbos                                                                                                                           =
//=====================================================================================================================================
void Renderer::Pps::Hdr::initFbos( Fbo& fbo, Texture& fai, int internalFormat )
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( w, h, internalFormat, GL_RGB );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage HDR passes FBO" );

	// unbind
	fbo.unbind();
}


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Renderer::Pps::Hdr::init()
{
	w = R::Pps::Hdr::renderingQuality * R::w;
	h = R::Pps::Hdr::renderingQuality * R::h;

	initFbos( pass0Fbo, pass0Fai, GL_RGB );
	initFbos( pass1Fbo, pass1Fai, GL_RGB );
	initFbos( pass2Fbo, fai, GL_RGB );

	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// init shaders
	//
	const char* shaderFname = "shaders/PpsHdr.glsl";

	if( !pass0SProg.customLoad( shaderFname, ("#define _PPS_HDR_PASS_0_\n#define IS_FAI_WIDTH " + Util::floatToStr(R::w) + "\n").c_str() ) )
		FATAL( "See prev error" );
	uniLocs.pass0SProg.fai = pass0SProg.getUniVar("fai")->getLoc();

	if( !pass1SProg.customLoad( shaderFname, ("#define _PPS_HDR_PASS_1_\n#define PASS0_HEIGHT " + Util::floatToStr(h) + "\n").c_str() ) )
		FATAL( "See prev error" );
	uniLocs.pass1SProg.fai = pass1SProg.getUniVar("fai")->getLoc();

	if( !pass2SProg.customLoad( shaderFname, "#define _PPS_HDR_PASS_2_\n" ) )
		FATAL( "See prev error" );
	uniLocs.pass2SProg.fai = pass2SProg.getUniVar("fai")->getLoc();
}


//=====================================================================================================================================
// runPass                                                                                                                            =
//=====================================================================================================================================
void Renderer::Pps::Hdr::run()
{
	int w = renderingQuality * r.width;
	int h = renderingQuality * r.height;
	r.setViewport( 0, 0, w, h );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );


	// pass 0
	pass0Fbo.bind();
	pass0SProg.bind();
	pass0SProg.locTexUnit( uniLocs.pass0SProg.fai, r.is.fai, 0 );
	r.drawQuad( 0 );


	// pass 1
	pass1Fbo.bind();
	pass1SProg.bind();
	pass1SProg.locTexUnit( uniLocs.pass1SProg.uniLocs.fai, pass0Fai, 0 );
	r.drawQuad( 0 );


	// pass 2
	pass2Fbo.bind();
	pass2SProg.bind();
	pass2SProg.locTexUnit( uniLocs.pass2SProg.fai, pass1Fai, 0 );
	r.drawQuad( 0 );

	// end
	Fbo::unbind();
}
