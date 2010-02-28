#include "Renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Fbo.h"

namespace R {
namespace Pps {

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
static ShaderProg* sProg;

namespace shdrVars
{
	int isFai;
	int ppsSsaoFai;
	int msNormalFai;
	int hdrFai;
	int lscattFai;
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
	fai.createEmpty2D( R::w, R::h, GL_RGB, GL_RGB );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create post-processing stage FBO" );

	fbo.Unbind();


	// init the shader and it's vars
	sProg = rsrc::shaders.load( "shaders/pps.glsl" );
	sProg->bind();

	shdrVars::isFai = sProg->getUniLoc( "is_fai" );

	if( R::Pps::Ssao::enabled )
	{
		R::Pps::Ssao::init();
		shdrVars::ppsSsaoFai = sProg->getUniLoc( "pps_ssao_fai" );
	}

	if( R::Pps::Hdr::enabled )
	{
		R::Pps::Hdr::init();
		shdrVars::hdrFai = sProg->getUniLoc( "pps_hdr_fai" );
	}

	if( R::Pps::edgeaa::enabled )
		shdrVars::msNormalFai = sProg->getUniLoc( "ms_normal_fai" );

	if( R::Pps::Lscatt::enabled )
	{
		R::Pps::Lscatt::init();
		shdrVars::lscattFai = sProg->getUniLoc( "pps_lscatt_fai" );
	}

}


/*
=======================================================================================================================================
runStage                                                                                                                              =
=======================================================================================================================================
*/
void runStage( const Camera& cam )
{
	if( R::Pps::Ssao::enabled )
		R::Pps::Ssao::runPass( cam );

	if( R::Pps::Hdr::enabled )
		R::Pps::Hdr::runPass( cam );

	if( R::Pps::Lscatt::enabled )
		R::Pps::Lscatt::runPass( cam );

	fbo.bind();

	// set GL
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	R::setViewport( 0, 0, R::w, R::h );

	// set shader
	sProg->bind();

	R::Is::fai.bind(0);
	//R::Ms::depthFai.bind(0);
	glUniform1i( shdrVars::isFai, 0 );

	if( R::Pps::Ssao::enabled )
	{
		R::Pps::Ssao::bluredFai2.bind(1);
		glUniform1i( shdrVars::ppsSsaoFai, 1 );
	}

	if( R::Pps::edgeaa::enabled )
	{
		R::Ms::normalFai.bind(2);
		glUniform1i( shdrVars::msNormalFai, 2 );
	}

	if( R::Pps::Hdr::enabled )
	{
		R::Pps::Hdr::pass2Fai.bind(3);
		//R::Bs::r_fai.bind(3);
		glUniform1i( shdrVars::hdrFai, 3 );
	}

	if( R::Pps::Lscatt::enabled )
	{
		R::Pps::Lscatt::fai.bind(4);
		glUniform1i( shdrVars::lscattFai, 4 );
	}


	// draw quad
	R::DrawQuad( sProg->getAttribLoc(0) );

	// unbind FBO
	fbo.Unbind();
}


}} // end namespaces

