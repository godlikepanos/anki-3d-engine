/*
The file contains functions and vars used for the deferred shading/post-processing stage/SSAO pass.
*/

#include "renderer.h"
#include "resource.h"
#include "texture.h"
#include "scene.h"
#include "r_private.h"
#include "fbo.h"

namespace r {
namespace pps {
namespace ssao {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static fbo_t fbo, blur_fbo; // yet another FBO

float rendering_quality = 0.25; // the rendering_quality of the SSAO fai. Chose low so it can blend
bool enabled = true;

static uint wwidth, wheight; // window width and height

texture_t fai, blured_fai; // SSAO factor

static shader_prog_t* shdr_ppp_ssao, * blur_shdr;

static texture_t* noise_map;


//=====================================================================================================================================
// InitBlurFBO                                                                                                                        =
//=====================================================================================================================================
static void InitBlurFBO()
{
	// create FBO
	blur_fbo.Create();
	blur_fbo.Bind();

	// inform in what buffers we draw
	blur_fbo.SetNumOfColorAttachements(1);

	// create the texes
	blured_fai.CreateEmpty( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	blured_fai.TexParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	blured_fai.TexParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, blured_fai.GetGLID(), 0 );

	// test if success
	if( !blur_fbo.IsGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO blur FBO" );

	// unbind
	blur_fbo.Unbind();
}


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	if( rendering_quality<0.0 || rendering_quality>1.0 ) ERROR("Incorect r::pps:ssao::rendering_quality");
	wwidth = r::rendering_quality * r::pps::ssao::rendering_quality * r::w;
	wheight = r::rendering_quality * r::pps::ssao::rendering_quality * r::h;

	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(1);

	// create the texes
	fai.CreateEmpty( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	fai.TexParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	fai.TexParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.GetGLID(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO pass FBO" );

	// unbind
	fbo.Unbind();


	// init shaders
	shdr_ppp_ssao = rsrc::shaders.Load( "shaders/pps_ssao.glsl" );

	// load noise map and disable temporaly the texture compression and enable mipmaping
	bool tex_compr = r::texture_compression;
	bool mipmaping = r::mipmaping;
	r::texture_compression = false;
	r::mipmaping = true;
	noise_map = rsrc::textures.Load( "gfx/noise3.tga" );
	noise_map->TexParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	noise_map->TexParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );
	//noise_map->TexParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//noise_map->TexParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	r::texture_compression = tex_compr;
	r::mipmaping = mipmaping;

	// blur FBO
	InitBlurFBO();
	blur_shdr = rsrc::shaders.Load( "shaders/pps_ssao_blur.glsl" );
}


/*
=======================================================================================================================================
RunPass                                                                                                                               =
=======================================================================================================================================
*/
void RunPass( const camera_t& cam )
{
	fbo.Bind();

	r::SetViewport( 0, 0, wwidth, wheight );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	// fill SSAO FAI
	shdr_ppp_ssao->Bind();
	glUniform2fv( shdr_ppp_ssao->GetUniformLocation(0), 1, &(vec2_t(cam.GetZNear(), cam.GetZFar()))[0] );
	shdr_ppp_ssao->LocTexUnit( shdr_ppp_ssao->GetUniformLocation(1), ms::depth_fai, 0 );
	shdr_ppp_ssao->LocTexUnit( shdr_ppp_ssao->GetUniformLocation(2), *noise_map, 1 );
	shdr_ppp_ssao->LocTexUnit( shdr_ppp_ssao->GetUniformLocation(3), ms::normal_fai, 2 );
	r::DrawQuad( shdr_ppp_ssao->GetAttributeLocation(0) ); // Draw quad


	// second pass. blur
	blur_fbo.Bind();
	blur_shdr->Bind();
	blur_shdr->LocTexUnit( blur_shdr->GetUniformLocation(0), fai, 0 );
	r::DrawQuad( blur_shdr->GetAttributeLocation(0) ); // Draw quad

	// end
	fbo_t::Unbind();
}


}}} // end namespaces
