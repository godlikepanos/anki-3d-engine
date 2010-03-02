#include "Renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Scene.h"
#include "Fbo.h"

namespace R {
namespace Pps {
namespace Lscatt {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static Fbo fbo; // yet another FBO

float renderingQuality = 1.0;
bool enabled = false;

Texture fai;

static ShaderProg sProg;
static int msDepthFaiUniLoc;
static int isFaiUniLoc;


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	if( renderingQuality<0.0 || renderingQuality>1.0 ) ERROR("Incorect R::pps:lscatt::rendering_quality");
	float wwidth = R::Pps::Lscatt::renderingQuality * R::w;
	float wheight = R::Pps::Lscatt::renderingQuality * R::h;

	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( wwidth, wheight, GL_RGB, GL_RGB );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage light scattering pass FBO" );

	// unbind
	fbo.unbind();


	// init shaders
	sProg.customLoad( "shaders/pps_lscatt.glsl" );
	msDepthFaiUniLoc = sProg.getUniVar( "msDepthFai" ).getLoc();
	isFaiUniLoc = sProg.getUniVar( "isFai" ).getLoc();
}


/*
=======================================================================================================================================
runPass                                                                                                                               =
=======================================================================================================================================
*/
void runPass( const Camera& cam )
{
	fbo.bind();

	R::setViewport( 0, 0, R::w * renderingQuality, R::h * renderingQuality );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	// set the shader
	sProg.bind();

	sProg.locTexUnit( msDepthFaiUniLoc, R::Ms::depthFai, 0 );
	sProg.locTexUnit( isFaiUniLoc, R::Is::fai, 1 );

	// pass the light
	Vec4 p = Vec4( Scene::getSunPos(), 1.0 );
	p = cam.getProjectionMatrix() * (cam.getViewMatrix() * p);
	p /= p.w;
	p = p/2 + 0.5;
	glUniform2fv( sProg.getUniVar("lightPosScreenSpace").getLoc(), 1, &p[0] );

	// Draw quad
	R::DrawQuad( 0 );

	// end
	fbo.unbind();
}


}}} // end namespaces

