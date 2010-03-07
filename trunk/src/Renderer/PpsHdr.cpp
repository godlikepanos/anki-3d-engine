/*
The file contains functions and vars used for the deferred shading/post-processing stage/bloom passes.
*/

#include "Renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Scene.h"
#include "Fbo.h"

namespace R {
namespace Pps {
namespace Hdr {


//=====================================================================================================================================
// VARS                                                                                                                               =
//=====================================================================================================================================
bool enabled = true;

static Fbo pass0Fbo, pass1Fbo, pass2Fbo; // yet another FBO and another, damn

float renderingQuality = 0.5; // 1/4 of the image
static uint w, h; // render width and height

// hdr images
Texture pass0Fai; // for vertical blur pass
Texture pass1Fai; // with the horizontal blur
Texture fai; ///< The final fai

class HdrShaderProg: public ShaderProg
{
	public:
		struct
		{
			int fai;
		} uniLocs;
};

static HdrShaderProg pass0SProg, pass1SProg, pass2SProg;


//=====================================================================================================================================
// initFbos                                                                                                                           =
//=====================================================================================================================================
static void initFbos( Fbo& fbo, Texture& fai, int internalFormat )
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( w, h, internalFormat, GL_RGB );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
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
void init()
{
	w = R::Pps::Hdr::renderingQuality * R::w;
	h = R::Pps::Hdr::renderingQuality * R::h;

	initFbos( pass0Fbo, pass0Fai, GL_RGB );
	initFbos( pass1Fbo, pass1Fai, GL_RGB );
	initFbos( pass2Fbo, fai, GL_RGB );

	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// init shaders
	if( !pass0SProg.customLoad( "shaders/PpsHdr.glsl", ("#define _PPS_HDR_PASS_0_\n#define IS_FAI_WIDTH " + Util::floatToStr(R::w) + "\n").c_str() ) )
		FATAL( "See prev error" );
	pass0SProg.uniLocs.fai = pass0SProg.getUniVar("fai").getLoc();

	if( !pass1SProg.customLoad( "shaders/PpsHdr.glsl", ("#define _PPS_HDR_PASS_1_\n#define PASS0_HEIGHT " + Util::floatToStr(h) + "\n").c_str() ) )
		FATAL( "See prev error" );
	pass1SProg.uniLocs.fai = pass1SProg.getUniVar("fai").getLoc();

	if( !pass2SProg.customLoad( "shaders/PpsHdr.glsl", "#define _PPS_HDR_PASS_2_\n" ) )
		FATAL( "See prev error" );
	pass2SProg.uniLocs.fai = pass2SProg.getUniVar("fai").getLoc();
}


//=====================================================================================================================================
// runPass                                                                                                                            =
//=====================================================================================================================================
void runPass( const Camera& /*cam*/ )
{
	R::setViewport( 0, 0, w, h );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );


	// pass 0
	pass0Fbo.bind();
	pass0SProg.bind();
	pass0SProg.locTexUnit( pass0SProg.uniLocs.fai, R::Is::fai, 0 );
	R::DrawQuad( 0 );


	// pass 1
	pass1Fbo.bind();
	pass1SProg.bind();
	pass1SProg.locTexUnit( pass1SProg.uniLocs.fai, pass0Fai, 0 );
	R::DrawQuad( 0 );


	// pass 2
	pass2Fbo.bind();
	pass2SProg.bind();
	pass2SProg.locTexUnit( pass2SProg.uniLocs.fai, pass1Fai, 0 );
	R::DrawQuad( 0 );

	// end
	Fbo::unbind();
}


}}} // end namespaces

