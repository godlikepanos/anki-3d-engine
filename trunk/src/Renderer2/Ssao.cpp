#include "Renderer.hpp"
#include "Camera.h"


//=====================================================================================================================================
// initBlurFbos                                                                                                                       =
//=====================================================================================================================================
void Renderer::Pps::Saao::initBlurFbo( Fbo& fbo, Texture& fai )
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements( 1 );

	// create the texes
	fai.createEmpty2D( bwidth, bheight, GL_ALPHA8, GL_ALPHA );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO blur FBO" );

	// unbind
	fbo.unbind();
}


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Renderer::Pps::Saao::init()
{
	width = renderingQuality * r.width;
	height = renderingQuality * r.height;
	bwidth = height * bluringQuality;
	bheight = height * bluringQuality;


	//
	// init FBOs
	//

	// create FBO
	pass0Fbo.create();
	pass0Fbo.bind();

	// inform in what buffers we draw
	pass0Fbo.setNumOfColorAttachements(1);

	// create the FAI
	pass0Fai.createEmpty2D( width, height, GL_ALPHA8, GL_ALPHA );
	pass0Fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	pass0Fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pass0Fai.getGlId(), 0 );

	// test if success
	if( !pass0Fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO pass FBO" );

	// unbind
	pass0Fbo.unbind();

	initBlurFbo( pass1Fbo, pass1Fai );
	initBlurFbo( pass2Fbo, fai );


	//
	// Shaders
	//

	ssaoSProg.customLoad( "shaders/PpsSsao.glsl" );
	blurSProg.customLoad( "shaders/PpsSsaoBlur.glsl", ("#define _PPS_SSAO_PASS_0_\n#define PASS0_FAI_WIDTH " + Util::floatToStr(width) + "\n").c_str() );
	blurSProg2.customLoad( "shaders/PpsSsaoBlur.glsl", ("#define _PPS_SSAO_PASS_1_\n#define PASS1_FAI_HEIGHT " + Util::floatToStr(bheight) + "\n").c_str() );

	uniLocs.pass0SProg.camerarange = ssaoSProg.getUniVar("camerarange")->getLoc();
	uniLocs.pass0SProg.msDepthFai = ssaoSProg.getUniVar("msDepthFai")->getLoc();
	uniLocs.pass0SProg.noiseMap = ssaoSProg.getUniVar("noiseMap")->getLoc();
	uniLocs.pass0SProg.msNormalFai = ssaoSProg.getUniVar("msNormalFai")->getLoc();
	uniLocs.pass1SProg.fai = blurSProg.getUniVar("tex")->getLoc(); /// @todo rename the tex in the shader
	uniLocs.pass2SProg.fai = blurSProg2.getUniVar("tex")->getLoc(); /// @todo rename the tex in the shader


	//
	// noise map
	//

	// load noise map and disable temporally the texture compression and enable mipmapping
	bool texCompr = Renderer::textureCompression;
	bool mipmaping = Renderer::mipmapping;
	Renderer::textureCompression = false;
	Renderer::mipmapping = true;
	noiseMap = Rsrc::textures.load( "gfx/noise3.tga" );
	noiseMap->texParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	noiseMap->texParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );
	//noise_map->texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//noise_map->texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	Renderer::textureCompression = texCompr;
	Renderer::mipmapping = mipmaping;

}


//=====================================================================================================================================
// run                                                                                                                                =
//=====================================================================================================================================
void Renderer::Pps::Saao::run()
{
	const Camera& cam = *r.cam;

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );


	// 1st pass
	r.setViewport( 0, 0, width, height );
	pass0Fbo.bind();
	ssaoSProg.bind();
	glUniform2fv( uniLocs.pass0SProg.camerarange, 1, &(Vec2(cam.getZNear(), cam.getZFar()))[0] );
	ssaoSProg.locTexUnit( uniLocs.pass0SProg.msDepthFai, r.ms.depthFai, 0 );
	ssaoSProg.locTexUnit( uniLocs.pass0SProg.noiseMap, *noiseMap, 1 );
	ssaoSProg.locTexUnit( uniLocs.pass0SProg.msNormalFai, r.ms.normalFai, 2 );
	r.drawQuad( 0 );

	// for 2nd and 3rd passes
	r.setViewport( 0, 0, bwidth, bheight );

	// 2nd pass
	pass1Fbo.bind();
	blurSProg.bind();
	blurSProg.locTexUnit( uniLocs.pass1SProg.fai, pass0Fai, 0 );
	r.drawQuad( 0 );

	// 3rd pass
	pass2Fbo.bind();
	blurSProg2.bind();
	blurSProg2.locTexUnit( uniLocs.pass2SProg.fai, pass1Fai, 0 );
	r.drawQuad( 0 );

	// end
	Fbo::unbind();
}

