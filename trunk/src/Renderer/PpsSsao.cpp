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
static Fbo pass0Fbo, pass1Fbo, pass2Fbo;

float renderingQuality = 0.20; // the renderingQuality of the SSAO fai. Chose low so it can blend
bool enabled = true;

static uint wwidth, wheight; // window width and height

Texture pass0Fai, pass1Fai, fai;

static ShaderProg ssaoSProg, blurSProg, blurSProg2;

static Texture* noise_map;


//=====================================================================================================================================
// initBlurFbos                                                                                                                       =
//=====================================================================================================================================
static void initBlurFbos()
{
	// create FBO
	pass1Fbo.create();
	pass1Fbo.bind();

	// inform in what buffers we draw
	pass1Fbo.setNumOfColorAttachements(1);

	// create the texes
	pass1Fai.createEmpty2D( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	pass1Fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	pass1Fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pass1Fai.getGlId(), 0 );

	// test if success
	if( !pass1Fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO blur FBO" );

	// unbind
	pass1Fbo.unbind();



	// create FBO
	pass2Fbo.create();
	pass2Fbo.bind();

	// inform in what buffers we draw
	pass2Fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !pass2Fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO blur FBO" );

	// unbind
	pass2Fbo.unbind();
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
	pass0Fbo.create();
	pass0Fbo.bind();

	// inform in what buffers we draw
	pass0Fbo.setNumOfColorAttachements(1);

	// create the texes
	pass0Fai.createEmpty2D( wwidth, wheight, GL_ALPHA8, GL_ALPHA );
	pass0Fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	pass0Fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pass0Fai.getGlId(), 0 );

	// test if success
	if( !pass0Fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO pass FBO" );

	// unbind
	pass0Fbo.unbind();


	// init shaders
	ssaoSProg.customLoad( "shaders/PpsSsao.glsl" );

	// load noise map and disable temporaly the texture compression and enable mipmaping
	bool texCompr = R::textureCompression;
	bool mipmaping = R::mipmaping;
	R::textureCompression = false;
	R::mipmaping = true;
	noise_map = Rsrc::textures.load( "gfx/noise3.tga" );
	noise_map->texParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	noise_map->texParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );
	//noise_map->texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//noise_map->texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	R::textureCompression = texCompr;
	R::mipmaping = mipmaping;

	// blur FBO
	initBlurFbos();
	blurSProg.customLoad( "shaders/PpsSsaoBlur.glsl", "#define _PPS_SSAO_PASS_0_\n" );
	blurSProg2.customLoad( "shaders/PpsSsaoBlur.glsl", "#define _PPS_SSAO_PASS_1_\n" );
}


/*
=======================================================================================================================================
runPass                                                                                                                               =
=======================================================================================================================================
*/
void runPass( const Camera& cam )
{
	pass0Fbo.bind();

	R::setViewport( 0, 0, wwidth, wheight );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	// fill SSAO FAI
	ssaoSProg.bind();
	glUniform2fv( ssaoSProg.getUniVar("camerarange").getLoc(), 1, &(Vec2(cam.getZNear(), cam.getZFar()))[0] );
	ssaoSProg.locTexUnit( ssaoSProg.getUniVar("msDepthFai").getLoc(), R::Ms::depthFai, 0 );
	ssaoSProg.locTexUnit( ssaoSProg.getUniVar("noiseMap").getLoc(), *noise_map, 1 );
	ssaoSProg.locTexUnit( ssaoSProg.getUniVar("msNormalFai").getLoc(), R::Ms::normalFai, 2 );
	R::DrawQuad( 0 ); // Draw quad


	// second pass. blur
	pass1Fbo.bind();
	blurSProg.bind();
	blurSProg.locTexUnit( blurSProg.getUniVar("tex").getLoc(), pass0Fai, 0 );
	R::DrawQuad( 0 ); // Draw quad

	// third pass. blur
	pass2Fbo.bind();
	blurSProg2.bind();
	blurSProg2.locTexUnit( blurSProg2.getUniVar("tex").getLoc(), pass1Fai, 0 );
	R::DrawQuad( 0 ); // Draw quad


	// end
	Fbo::unbind();
}


}}} // end namespaces
