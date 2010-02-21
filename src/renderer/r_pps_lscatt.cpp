#include "renderer.h"
#include "resource.h"
#include "texture.h"
#include "scene.h"
#include "r_private.h"
#include "fbo.h"

namespace r {
namespace pps {
namespace lscatt {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static fbo_t fbo; // yet another FBO

float rendering_quality = 1.0;
bool enabled = false;

texture_t fai;

static shader_prog_t* shdr;
static int ms_depth_fai_uni_loc;
static int is_fai_uni_loc;


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	if( rendering_quality<0.0 || rendering_quality>1.0 ) ERROR("Incorect r::pps:lscatt::rendering_quality");
	float wwidth = r::pps::lscatt::rendering_quality * r::w;
	float wheight = r::pps::lscatt::rendering_quality * r::h;

	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(1);

	// create the texes
	fai.CreateEmpty2D( wwidth, wheight, GL_RGB, GL_RGB );
	fai.TexParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.TexParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.GetGLID(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create deferred shading post-processing stage light scattering pass FBO" );

	// unbind
	fbo.Unbind();


	// init shaders
	shdr = rsrc::shaders.Load( "shaders/pps_lscatt.glsl" );
	ms_depth_fai_uni_loc = shdr->GetUniformLocation( "ms_depth_fai" );
	is_fai_uni_loc = shdr->GetUniformLocation( "is_fai" );
}


/*
=======================================================================================================================================
RunPass                                                                                                                               =
=======================================================================================================================================
*/
void RunPass( const camera_t& cam )
{
	fbo.Bind();

	r::SetViewport( 0, 0, r::w * rendering_quality, r::h * rendering_quality );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	// set the shader
	shdr->Bind();

	shdr->LocTexUnit( ms_depth_fai_uni_loc, r::ms::depth_fai, 0 );
	shdr->LocTexUnit( is_fai_uni_loc, r::is::fai, 1 );

	// pass the light
	vec4_t p = vec4_t( scene::SunPos(), 1.0 );
	p = cam.GetProjectionMatrix() * (cam.GetViewMatrix() * p);
	p /= p.w;
	p = p/2 + 0.5;
	glUniform2fv( shdr->GetUniformLocation("light_pos_screen_space"), 1, &p[0] );

	// Draw quad
	r::DrawQuad( shdr->GetAttributeLocation(0) );

	// end
	fbo.Unbind();
}


}}} // end namespaces

