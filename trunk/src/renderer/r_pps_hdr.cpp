/*
The file contains functions and vars used for the deferred shading/post-processing stage/bloom passes.
*/

#include "renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "scene.h"
#include "r_private.h"
#include "fbo.h"

namespace r {
namespace pps {
namespace hdr {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
bool enabled = true;

static fbo_t pass0_fbo, pass1_fbo, pass2_fbo; // yet another FBO and another, damn

float rendering_quality = 0.25; // 1/4 of the image
static uint wwidth, wheight; // render width and height

// hdr images
Texture pass0_fai; // for vertical blur pass
Texture pass1_fai; // with the horizontal blur
Texture pass2_fai;

static ShaderProg* pass0_shdr, * pass1_shdr, * pass2_shdr; // hdr pass 0 and pass 1 shaders


/*
=======================================================================================================================================
InitFBOs                                                                                                                              =
=======================================================================================================================================
*/
static void InitFBOs( fbo_t& fbo, Texture& fai, int internal_format )
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( wwidth, wheight, internal_format, GL_RGB );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create deferred shading post-processing stage HDR passes FBO" );

	// unbind
	fbo.Unbind();
}


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	wwidth = r::pps::hdr::rendering_quality * r::w;
	wheight = r::pps::hdr::rendering_quality * r::h;

	InitFBOs( pass0_fbo, pass0_fai, GL_RGB );
	InitFBOs( pass1_fbo, pass1_fai, GL_RGB );
	InitFBOs( pass2_fbo, pass2_fai, GL_RGB );

	// init shaders
	pass0_shdr = rsrc::shaders.load( "shaders/pps_hdr_pass0.glsl" );
	pass1_shdr = rsrc::shaders.load( "shaders/pps_hdr_pass1.glsl" );
	pass2_shdr = rsrc::shaders.load( "shaders/pps_hdr_pass2.glsl" );
}


/*
=======================================================================================================================================
RunPass                                                                                                                               =
=======================================================================================================================================
*/
void RunPass( const camera_t& /*cam*/ )
{
	r::SetViewport( 0, 0, wwidth, wheight );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );


	// pass 0
	pass0_fbo.Bind();

	pass0_shdr->bind();

	pass0_shdr->locTexUnit( pass0_shdr->GetUniLoc(0), r::is::fai, 0 );

	// Draw quad
	r::DrawQuad( pass0_shdr->getAttribLoc(0) );


	// pass 1
	pass1_fbo.Bind();

	pass1_shdr->bind();

	pass1_shdr->locTexUnit( pass1_shdr->GetUniLoc(0), pass0_fai, 0 );

	// Draw quad
	r::DrawQuad( pass1_shdr->getAttribLoc(0) );


	// pass 2
	pass2_fbo.Bind();

	pass2_shdr->bind();

	pass2_shdr->locTexUnit( pass2_shdr->GetUniLoc(0), pass1_fai, 0 );

	// Draw quad
	r::DrawQuad( pass2_shdr->getAttribLoc(0) );

	// end
	fbo_t::Unbind();
}


}}} // end namespaces

