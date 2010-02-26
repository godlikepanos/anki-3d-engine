#include "renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Scene.h"
#include "Fbo.h"

namespace r {
namespace pps {
namespace lscatt {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static Fbo fbo; // yet another FBO

float renderingQuality = 1.0;
bool enabled = false;

Texture fai;

static ShaderProg* shdr;
static int ms_depth_fai_uni_loc;
static int is_fai_uni_loc;


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	if( renderingQuality<0.0 || renderingQuality>1.0 ) ERROR("Incorect r::pps:lscatt::rendering_quality");
	float wwidth = r::pps::lscatt::renderingQuality * r::w;
	float wheight = r::pps::lscatt::renderingQuality * r::h;

	// create FBO
	fbo.Create();
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
	fbo.Unbind();


	// init shaders
	shdr = rsrc::shaders.load( "shaders/pps_lscatt.glsl" );
	ms_depth_fai_uni_loc = shdr->getUniLoc( "ms_depth_fai" );
	is_fai_uni_loc = shdr->getUniLoc( "is_fai" );
}


/*
=======================================================================================================================================
runPass                                                                                                                               =
=======================================================================================================================================
*/
void runPass( const Camera& cam )
{
	fbo.bind();

	r::setViewport( 0, 0, r::w * renderingQuality, r::h * renderingQuality );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	// set the shader
	shdr->bind();

	shdr->locTexUnit( ms_depth_fai_uni_loc, r::ms::depthFai, 0 );
	shdr->locTexUnit( is_fai_uni_loc, r::is::fai, 1 );

	// pass the light
	Vec4 p = Vec4( Scene::getSunPos(), 1.0 );
	p = cam.getProjectionMatrix() * (cam.getViewMatrix() * p);
	p /= p.w;
	p = p/2 + 0.5;
	glUniform2fv( shdr->getUniLoc("light_pos_screen_space"), 1, &p[0] );

	// Draw quad
	r::DrawQuad( shdr->getAttribLoc(0) );

	// end
	fbo.Unbind();
}


}}} // end namespaces

