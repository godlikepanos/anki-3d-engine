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


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
bool enabled = true;

static Fbo pass0_fbo, pass1_fbo, pass2_fbo; // yet another FBO and another, damn

float renderingQuality = 0.25; // 1/4 of the image
static uint wwidth, wheight; // render width and height

// hdr images
Texture pass0Fai; // for vertical blur pass
Texture pass1Fai; // with the horizontal blur
Texture pass2Fai;

static ShaderProg* pass0_shdr, * pass1_shdr, * pass2_shdr; // hdr pass 0 and pass 1 shaders


/*
=======================================================================================================================================
InitFBOs                                                                                                                              =
=======================================================================================================================================
*/
static void InitFBOs( Fbo& fbo, Texture& fai, int internal_format )
{
	// create FBO
	fbo.Create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( wwidth, wheight, internal_format, GL_RGB );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage HDR passes FBO" );

	// unbind
	fbo.Unbind();
}


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	wwidth = R::Pps::Hdr::renderingQuality * R::w;
	wheight = R::Pps::Hdr::renderingQuality * R::h;

	InitFBOs( pass0_fbo, pass0Fai, GL_RGB );
	InitFBOs( pass1_fbo, pass1Fai, GL_RGB );
	InitFBOs( pass2_fbo, pass2Fai, GL_RGB );

	// init shaders
	pass0_shdr = rsrc::shaders.load( "shaders/pps_hdr_pass0.glsl" );
	pass1_shdr = rsrc::shaders.load( "shaders/pps_hdr_pass1.glsl" );
	pass2_shdr = rsrc::shaders.load( "shaders/pps_hdr_pass2.glsl" );
}


/*
=======================================================================================================================================
runPass                                                                                                                               =
=======================================================================================================================================
*/
void runPass( const Camera& /*cam*/ )
{
	R::setViewport( 0, 0, wwidth, wheight );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );


	// pass 0
	pass0_fbo.bind();

	pass0_shdr->bind();

	pass0_shdr->locTexUnit( pass0_shdr->getUniLoc(0), R::Is::fai, 0 );

	// Draw quad
	R::DrawQuad( pass0_shdr->getAttribLoc(0) );


	// pass 1
	pass1_fbo.bind();

	pass1_shdr->bind();

	pass1_shdr->locTexUnit( pass1_shdr->getUniLoc(0), pass0Fai, 0 );

	// Draw quad
	R::DrawQuad( pass1_shdr->getAttribLoc(0) );


	// pass 2
	pass2_fbo.bind();

	pass2_shdr->bind();

	pass2_shdr->locTexUnit( pass2_shdr->getUniLoc(0), pass1Fai, 0 );

	// Draw quad
	R::DrawQuad( pass2_shdr->getAttribLoc(0) );

	// end
	Fbo::Unbind();
}


}}} // end namespaces

