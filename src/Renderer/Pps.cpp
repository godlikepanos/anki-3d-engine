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

class PpsShaderProg: public ShaderProg
{
	public:
		struct
		{
			int isFai;
			int ppsSsaoFai;
			int msNormalFai;
			int hdrFai;
			int lscattFai;
		} uniLocs;
};

static PpsShaderProg sProg;


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	// create FBO
	fbo.create();
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

	fbo.unbind();


	// init the shader and it's vars
	sProg.customLoad( "shaders/Pps.glsl" );
	sProg.bind();

	sProg.uniLocs.isFai = sProg.findUniVar( "isFai" )->getLoc();

	if( R::Pps::Ssao::enabled )
	{
		R::Pps::Ssao::init();
		sProg.uniLocs.ppsSsaoFai = sProg.findUniVar( "ppsSsaoFai" )->getLoc();
	}

	if( R::Pps::Hdr::enabled )
	{
		R::Pps::Hdr::init();
		sProg.uniLocs.hdrFai = sProg.findUniVar( "ppsHdrFai" )->getLoc();
	}

	if( R::Pps::edgeaa::enabled )
		sProg.uniLocs.msNormalFai = sProg.findUniVar( "msNormalFai" )->getLoc();

	if( R::Pps::Lscatt::enabled )
	{
		R::Pps::Lscatt::init();
		sProg.uniLocs.lscattFai = sProg.findUniVar( "ppsLscattFai" )->getLoc();
	}

}


//=====================================================================================================================================
// runStage                                                                                                                           =
//=====================================================================================================================================
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
	sProg.bind();

	sProg.locTexUnit( sProg.uniLocs.isFai, R::Is::fai, 0 );

	if( R::Pps::Ssao::enabled )
	{
		sProg.locTexUnit( sProg.uniLocs.ppsSsaoFai, R::Pps::Ssao::fai, 1 );
	}

	if( R::Pps::edgeaa::enabled )
	{
		sProg.locTexUnit( sProg.uniLocs.msNormalFai, R::Ms::normalFai, 2 );
	}

	if( R::Pps::Hdr::enabled )
	{
		sProg.locTexUnit( sProg.uniLocs.hdrFai, R::Pps::Hdr::fai, 3 );
	}

	if( R::Pps::Lscatt::enabled )
	{
		sProg.locTexUnit( sProg.uniLocs.lscattFai, R::Pps::Lscatt::fai, 4 );
	}


	// draw quad
	R::DrawQuad( 0 );

	// unbind FBO
	fbo.unbind();
}


}} // end namespaces

