/*
The file contains functions and vars used for the deferred shading/post-processing stage/bloom passes.
*/

#include "renderer.h"
#include "resource.h"
#include "texture.h"
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
texture_t pass0_fai; // for vertical blur pass
texture_t pass1_fai; // with the horizontal blur
texture_t pass2_fai;

static shader_prog_t* pass0_shdr, * pass1_shdr, * pass2_shdr; // hdr pass 0 and pass 1 shaders


/*
=======================================================================================================================================
InitFBOs                                                                                                                              =
=======================================================================================================================================
*/
static void InitFBOs( fbo_t& fbo, texture_t& fai, int internal_format )
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(1);

	// create the texes
	fai.CreateEmpty( wwidth, wheight, internal_format, GL_RGB );
	fai.TexParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.TexParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.GetGLID(), 0 );

	// test if success
	if( !fbo.CheckStatus() )
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
	wwidth = r::rendering_quality * r::pps::hdr::rendering_quality * r::w;
	wheight = r::rendering_quality * r::pps::hdr::rendering_quality * r::h;

	InitFBOs( pass0_fbo, pass0_fai, GL_RGB );
	InitFBOs( pass1_fbo, pass1_fai, GL_RGB );
	InitFBOs( pass2_fbo, pass2_fai, GL_RGB );

	// init shaders
	pass0_shdr = rsrc::shaders.Load( "shaders/pps_hdr_pass0.glsl" );
	pass1_shdr = rsrc::shaders.Load( "shaders/pps_hdr_pass1.glsl" );
	pass2_shdr = rsrc::shaders.Load( "shaders/pps_hdr_pass2.glsl" );
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

	pass0_shdr->Bind();

	pass0_shdr->LocTexUnit( pass0_shdr->GetUniformLocation(0), r::is::fai, 0 );

	// Draw quad
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );
	glDrawArrays( GL_QUADS, 0, 4 );
	glDisableClientState( GL_VERTEX_ARRAY );


	// pass 1
	pass1_fbo.Bind();

	pass1_shdr->Bind();

	pass1_shdr->LocTexUnit( pass1_shdr->GetUniformLocation(0), pass0_fai, 0 );

	// Draw quad
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );
	glDrawArrays( GL_QUADS, 0, 4 );
	glDisableClientState( GL_VERTEX_ARRAY );


	// pass 2
	pass2_fbo.Bind();

	pass2_shdr->Bind();

	pass2_shdr->LocTexUnit( pass2_shdr->GetUniformLocation(0), pass1_fai, 0 );

	// Draw quad
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );
	glDrawArrays( GL_QUADS, 0, 4 );
	glDisableClientState( GL_VERTEX_ARRAY );


	// end
	fbo_t::Unbind();
}


}}} // end namespaces

