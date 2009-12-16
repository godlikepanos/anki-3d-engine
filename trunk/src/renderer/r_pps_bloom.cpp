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
namespace bloom {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
bool enabled = true;

static fbo_t vblur_fbo, final_fbo; // yet another FBO and another, damn

float rendering_quality = 0.25; // 1/4 of the image
static uint wwidth, wheight; // render width and height

// bloom images
texture_t vbrur_fai; // for vertical blur pass
texture_t final_fai; // with the horizontal blur

static shader_prog_t* shdr_pps_bloom_vblur, * shdr_pps_bloom_final; // bloom pass 0 and pass 1 shaders


/*
=======================================================================================================================================
InitFBOP0                                                                                                                             =
=======================================================================================================================================
*/
static void InitFBOs( fbo_t& fbo, texture_t& fai )
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(1);

	// create the texes
	// we need one chanel onle (red) because the output is grayscale
	//fai_bloom.CreateEmpty( wwidth, wheight, 1, GL_RED, GL_UNSIGNED_BYTE ); // ORIGINAL BLOOM CODE
	fai.CreateEmpty( wwidth, wheight, GL_RGB, GL_RGB );
	fai.TexParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.TexParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.GetGLID(), 0 );

	// test if success
	if( !fbo.CheckStatus() )
		FATAL( "Cannot create deferred shading post-processing stage bloom passes FBO" );

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
	DEBUG_ERR( rendering_quality<0.0 || rendering_quality>1.0 );
	wwidth = r::rendering_quality * r::pps::ssao::rendering_quality * r::w;
	wheight = r::rendering_quality * r::pps::ssao::rendering_quality * r::h;

	InitFBOs( vblur_fbo, vbrur_fai );
	InitFBOs( final_fbo, final_fai );

	// init shaders
	shdr_pps_bloom_vblur = rsrc::shaders.Load( "shaders/pps_bloom_vblur.glsl" );
	shdr_pps_bloom_final = rsrc::shaders.Load( "shaders/pps_bloom_final.glsl" );
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
	vblur_fbo.Bind();

	shdr_pps_bloom_vblur->Bind();

	shdr_pps_bloom_vblur->LocTexUnit( shdr_pps_bloom_vblur->GetUniformLocation(0), r::is::fai, 0 );

	// Draw quad
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );


	// pass 1
	final_fbo.Bind();

	shdr_pps_bloom_final->Bind();

	shdr_pps_bloom_final->LocTexUnit( shdr_pps_bloom_final->GetUniformLocation(0), vbrur_fai, 0 );

	// Draw quad
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );

	// end
	final_fbo.Unbind();
}


}}} // end namespaces

