/*
The file contains functions and vars used for the deferred shading/post-processing stage/SSAO pass.
*/

#include "Renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Scene.h"
#include "Fbo.h"
#include "Camera.h"

namespace R {
namespace Pps {
namespace Ssao {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static Fbo fbo, blurFbo, blurFbo2; // yet another FBO

float renderingQuality = 0.20; // the renderingQuality of the SSAO fai. Chose low so it can blend
bool enabled = true;

static uint wwidth, wheight; // window width and height

Texture fai, bluredFai, bluredFai2; // SSAO factor

static ShaderProg* ssaoSProg, * blurSProg, * blurSProg2;

static Texture* noise_map;


//=====================================================================================================================================
// InitBlurFBO                                                                                                                        =
//=====================================================================================================================================
static void InitBlurFBO()
{
	// create FBO
	blurFbo.Create();
	blurFbo.bind();

	// inform in what buffers we draw
	blurFbo.setNumOfColorAttachements(1);

	// create the texes
	bluredFai.createEmpty2D( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	//bluredFai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//bluredFai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, bluredFai.getGlId(), 0 );

	// test if success
	if( !blurFbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO blur FBO" );

	// unbind
	blurFbo.Unbind();



	// create FBO
	blurFbo2.Create();
	blurFbo2.bind();

	// inform in what buffers we draw
	blurFbo2.setNumOfColorAttachements(1);

	// create the texes
	bluredFai2.createEmpty2D( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	bluredFai2.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	bluredFai2.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, bluredFai2.getGlId(), 0 );

	// test if success
	if( !blurFbo2.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO blur FBO" );

	// unbind
	blurFbo2.Unbind();
}


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	if( renderingQuality<0.0 || renderingQuality>1.0 ) ERROR("Incorect R::pps:ssao::rendering_quality");
	wwidth = R::Pps::Ssao::renderingQuality * R::w;
	wheight = R::Pps::Ssao::renderingQuality * R::h;

	// create FBO
	fbo.Create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	//fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO pass FBO" );

	// unbind
	fbo.Unbind();


	// init shaders
	ssaoSProg = rsrc::shaders.load( "shaders/pps_ssao.glsl" );

	// load noise map and disable temporaly the texture compression and enable mipmaping
	bool tex_compr = R::textureCompression;
	bool mipmaping = R::mipmaping;
	R::textureCompression = false;
	R::mipmaping = true;
	noise_map = rsrc::textures.load( "gfx/noise3.tga" );
	noise_map->texParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	noise_map->texParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );
	//noise_map->texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//noise_map->texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	R::textureCompression = tex_compr;
	R::mipmaping = mipmaping;

	// blur FBO
	InitBlurFBO();
	blurSProg = rsrc::shaders.load( "shaders/pps_ssao_blur.glsl" );
	blurSProg2 = rsrc::shaders.load( "shaders/pps_ssao_blur2.glsl" );
}


/*
=======================================================================================================================================
runPass                                                                                                                               =
=======================================================================================================================================
*/
void runPass( const Camera& cam )
{
	fbo.bind();

	R::setViewport( 0, 0, wwidth, wheight );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	// fill SSAO FAI
	ssaoSProg->bind();
	glUniform2fv( ssaoSProg->getUniVar("camerarange").getLoc(), 1, &(Vec2(cam.getZNear(), cam.getZFar()))[0] );
	ssaoSProg->locTexUnit( ssaoSProg->getUniVar("msDepthFai").getLoc(), R::Ms::depthFai, 0 );
	ssaoSProg->locTexUnit( ssaoSProg->getUniVar("noiseMap").getLoc(), *noise_map, 1 );
	ssaoSProg->locTexUnit( ssaoSProg->getUniVar("msNormalFai").getLoc(), R::Ms::normalFai, 2 );
	R::DrawQuad( 0 ); // Draw quad


	// second pass. blur
	blurFbo.bind();
	blurSProg->bind();
	blurSProg->locTexUnit( blurSProg->getUniVar("tex").getLoc(), fai, 0 );
	R::DrawQuad( 0 ); // Draw quad

	// third pass. blur
	blurFbo2.bind();
	blurSProg2->bind();
	blurSProg2->locTexUnit( blurSProg2->getUniVar("tex").getLoc(), bluredFai, 0 );
	R::DrawQuad( 0 ); // Draw quad


	// end
	Fbo::Unbind();
}


}}} // end namespaces
