#include "renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Fbo.h"

namespace r {
namespace pps {

namespace edgeaa {
	bool enabled = false;
}

/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/

static Fbo fbo; // yet another FBO

Texture fai;

// shader stuff
static ShaderProg* shdr_post_proc_stage;

namespace shdr_vars
{
	int is_fai;
	int pps_ssao_fai;
	int ms_normal_fai;
	int hdr_fai;
	int lscatt_fai;
}


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	// create FBO
	fbo.Create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( r::w, r::h, GL_RGB, GL_RGB );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create post-processing stage FBO" );

	fbo.Unbind();


	// init the shader and it's vars
	shdr_post_proc_stage = rsrc::shaders.load( "shaders/pps.glsl" );
	shdr_post_proc_stage->bind();

	shdr_vars::is_fai = shdr_post_proc_stage->getUniLoc( "is_fai" );

	if( r::pps::ssao::enabled )
	{
		r::pps::ssao::init();
		shdr_vars::pps_ssao_fai = shdr_post_proc_stage->getUniLoc( "pps_ssao_fai" );
	}

	if( r::pps::hdr::enabled )
	{
		r::pps::hdr::init();
		shdr_vars::hdr_fai = shdr_post_proc_stage->getUniLoc( "pps_hdr_fai" );
	}

	if( r::pps::edgeaa::enabled )
		shdr_vars::ms_normal_fai = shdr_post_proc_stage->getUniLoc( "ms_normal_fai" );

	if( r::pps::lscatt::enabled )
	{
		r::pps::lscatt::init();
		shdr_vars::lscatt_fai = shdr_post_proc_stage->getUniLoc( "pps_lscatt_fai" );
	}

}


/*
=======================================================================================================================================
runStage                                                                                                                              =
=======================================================================================================================================
*/
void runStage( const Camera& cam )
{
	if( r::pps::ssao::enabled )
		r::pps::ssao::runPass( cam );

	if( r::pps::hdr::enabled )
		r::pps::hdr::runPass( cam );

	if( r::pps::lscatt::enabled )
		r::pps::lscatt::runPass( cam );

	fbo.bind();

	// set GL
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	r::setViewport( 0, 0, r::w, r::h );

	// set shader
	shdr_post_proc_stage->bind();

	r::is::fai.bind(0);
	//r::ms::depthFai.bind(0);
	glUniform1i( shdr_vars::is_fai, 0 );

	if( r::pps::ssao::enabled )
	{
		r::pps::ssao::bluredFai.bind(1);
		glUniform1i( shdr_vars::pps_ssao_fai, 1 );
	}

	if( r::pps::edgeaa::enabled )
	{
		r::ms::normalFai.bind(2);
		glUniform1i( shdr_vars::ms_normal_fai, 2 );
	}

	if( r::pps::hdr::enabled )
	{
		r::pps::hdr::pass2Fai.bind(3);
		//r::bs::r_fai.bind(3);
		glUniform1i( shdr_vars::hdr_fai, 3 );
	}

	if( r::pps::lscatt::enabled )
	{
		r::pps::lscatt::fai.bind(4);
		glUniform1i( shdr_vars::lscatt_fai, 4 );
	}


	// draw quad
	r::DrawQuad( shdr_post_proc_stage->getAttribLoc(0) );

	// unbind FBO
	fbo.Unbind();
}


}} // end namespaces

