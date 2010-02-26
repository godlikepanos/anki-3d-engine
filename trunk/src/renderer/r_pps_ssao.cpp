/*
The file contains functions and vars used for the deferred shading/post-processing stage/SSAO pass.
*/

#include "renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Scene.h"
#include "Fbo.h"
#include "Camera.h"

namespace r {
namespace pps {
namespace ssao {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static Fbo fbo, blur_fbo; // yet another FBO

float renderingQuality = 0.25; // the renderingQuality of the SSAO fai. Chose low so it can blend
bool enabled = true;

static uint wwidth, wheight; // window width and height

Texture fai, bluredFai; // SSAO factor

static ShaderProg* shdr_ppp_ssao, * blur_shdr;

static Texture* noise_map;


//=====================================================================================================================================
// InitBlurFBO                                                                                                                        =
//=====================================================================================================================================
static void InitBlurFBO()
{
	// create FBO
	blur_fbo.Create();
	blur_fbo.bind();

	// inform in what buffers we draw
	blur_fbo.setNumOfColorAttachements(1);

	// create the texes
	bluredFai.createEmpty2D( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	bluredFai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	bluredFai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, bluredFai.getGlId(), 0 );

	// test if success
	if( !blur_fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO blur FBO" );

	// unbind
	blur_fbo.Unbind();
}


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	if( renderingQuality<0.0 || renderingQuality>1.0 ) ERROR("Incorect r::pps:ssao::rendering_quality");
	wwidth = r::pps::ssao::renderingQuality * r::w;
	wheight = r::pps::ssao::renderingQuality * r::h;

	// create FBO
	fbo.Create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO pass FBO" );

	// unbind
	fbo.Unbind();


	// init shaders
	shdr_ppp_ssao = rsrc::shaders.load( "shaders/pps_ssao.glsl" );

	// load noise map and disable temporaly the texture compression and enable mipmaping
	bool tex_compr = r::textureCompression;
	bool mipmaping = r::mipmaping;
	r::textureCompression = false;
	r::mipmaping = true;
	noise_map = rsrc::textures.load( "gfx/noise3.tga" );
	noise_map->texParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	noise_map->texParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );
	//noise_map->texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//noise_map->texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	r::textureCompression = tex_compr;
	r::mipmaping = mipmaping;

	// blur FBO
	InitBlurFBO();
	blur_shdr = rsrc::shaders.load( "shaders/pps_ssao_blur.glsl" );
}


/*
=======================================================================================================================================
runPass                                                                                                                               =
=======================================================================================================================================
*/
void runPass( const Camera& cam )
{
	fbo.bind();

	r::setViewport( 0, 0, wwidth, wheight );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	// fill SSAO FAI
	shdr_ppp_ssao->bind();
	glUniform2fv( shdr_ppp_ssao->GetUniLoc(0), 1, &(Vec2(cam.getZNear(), cam.getZFar()))[0] );
	shdr_ppp_ssao->locTexUnit( shdr_ppp_ssao->GetUniLoc(1), ms::depthFai, 0 );
	shdr_ppp_ssao->locTexUnit( shdr_ppp_ssao->GetUniLoc(2), *noise_map, 1 );
	shdr_ppp_ssao->locTexUnit( shdr_ppp_ssao->GetUniLoc(3), ms::normalFai, 2 );
	r::DrawQuad( shdr_ppp_ssao->getAttribLoc(0) ); // Draw quad


	// second pass. blur
	blur_fbo.bind();
	blur_shdr->bind();
	blur_shdr->locTexUnit( blur_shdr->GetUniLoc(0), fai, 0 );
	r::DrawQuad( blur_shdr->getAttribLoc(0) ); // Draw quad

	// end
	Fbo::Unbind();
}


}}} // end namespaces
