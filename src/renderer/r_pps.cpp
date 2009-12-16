#include "renderer.h"
#include "resource.h"
#include "texture.h"
#include "r_private.h"
#include "fbo.h"

namespace r {
namespace pps {

namespace edgeaa {
	bool enabled = true;
}

/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/

static fbo_t fbo; // yet another FBO

texture_t fai;

// shader stuff
static shader_prog_t* shdr_post_proc_stage;

namespace shdr_vars
{
	int is_fai;
	int pps_ssao_fai;
	int ms_normal_fai;
	int bloom_fai;
	int lscatt_fai;
}


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(1);

	// create the texes
	fai.CreateEmpty( r::w * r::rendering_quality, r::h * r::rendering_quality, GL_RGB, GL_RGB );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.GetGLID(), 0 );

	// test if success
	if( !fbo.CheckStatus() )
		FATAL( "Cannot create post-processing stage FBO" );

	fbo.Unbind();


	// init the shader and it's vars
	shdr_post_proc_stage = rsrc::shaders.Load( "shaders/pps.glsl" );
	shdr_post_proc_stage->Bind();

	shdr_vars::is_fai = shdr_post_proc_stage->GetUniformLocation( "is_fai" );

	if( r::pps::ssao::enabled )
	{
		r::pps::ssao::Init();
		shdr_vars::pps_ssao_fai = shdr_post_proc_stage->GetUniformLocation( "pps_ssao_fai" );
	}

	if( r::pps::bloom::enabled )
	{
		r::pps::bloom::Init();
		shdr_vars::bloom_fai = shdr_post_proc_stage->GetUniformLocation( "pps_boom_fai" );
	}

	if( r::pps::edgeaa::enabled )
		shdr_vars::ms_normal_fai = shdr_post_proc_stage->GetUniformLocation( "ms_normal_fai" );

	if( r::pps::lscatt::enabled )
	{
		r::pps::lscatt::Init();
		shdr_vars::lscatt_fai = shdr_post_proc_stage->GetUniformLocation( "pps_lscatt_fai" );
	}

}


/*
=======================================================================================================================================
RunStage                                                                                                                              =
=======================================================================================================================================
*/
void RunStage( const camera_t& cam )
{
	if( r::pps::ssao::enabled )
		r::pps::ssao::RunPass( cam );

	if( r::pps::bloom::enabled )
		r::pps::bloom::RunPass( cam );

	if( r::pps::lscatt::enabled )
		r::pps::lscatt::RunPass( cam );

	fbo.Bind();

	// set GL
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	glPolygonMode( GL_FRONT, GL_FILL );

	r::SetViewport( 0, 0, r::w*r::rendering_quality, r::h*r::rendering_quality );

	// set shader
	shdr_post_proc_stage->Bind();

	r::is::fai.Bind(0);
	glUniform1i( shdr_vars::is_fai, 0 );

	if( r::pps::ssao::enabled )
	{
		r::pps::ssao::fai.Bind(1);
		glUniform1i( shdr_vars::pps_ssao_fai, 1 );
	}

	if( r::pps::edgeaa::enabled )
	{
		r::ms::normal_fai.Bind(2);
		glUniform1i( shdr_vars::ms_normal_fai, 2 );
	}

	if( r::pps::bloom::enabled )
	{
		r::pps::bloom::final_fai.Bind(3);
		glUniform1i( shdr_vars::bloom_fai, 3 );
	}

	if( r::pps::lscatt::enabled )
	{
		r::pps::lscatt::fai.Bind(4);
		glUniform1i( shdr_vars::lscatt_fai, 4 );
	}


	// draw quad
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );

	// unbind FBO
	fbo.Unbind();
}


}} // end namespaces

