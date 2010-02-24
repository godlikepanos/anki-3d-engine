/**
 * The file contains functions and vars used for the deferred shading blending stage stage.
 * The blending stage comes after the illumination stage. All the objects that are transculent will be drawn here.
 */

#include "renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Mesh.h"
#include "r_private.h"
#include "Resource.h"
#include "fbo.h"
#include "MeshNode.h"
#include "Material.h"


namespace r {
namespace bs {

//=====================================================================================================================================
// VARS                                                                                                                               =
//=====================================================================================================================================
static fbo_t intermid_fbo, fbo;

static Texture fai; ///< RGB for color and A for mask (0 doesnt pass, 1 pass)
static ShaderProg* shader_prog;


//=====================================================================================================================================
// Init2                                                                                                                              =
//=====================================================================================================================================
void Init2()
{
	//** 1st FBO **
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform FBO about the color buffers
	fbo.SetNumOfColorAttachements(1);

	// attach the texes
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r::pps::fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, r::ms::depth_fai.getGlId(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	fbo.Unbind();


	//** 2nd FBO **
	intermid_fbo.Create();
	intermid_fbo.Bind();

	// texture
	intermid_fbo.SetNumOfColorAttachements(1);
	fai.createEmpty2D( r::w, r::h, GL_RGBA8, GL_RGBA );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// attach the texes
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, r::ms::depth_fai.getGlId(), 0 );

	// test if success
	if( !intermid_fbo.IsGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	intermid_fbo.Unbind();

	shader_prog = rsrc::shaders.load( "shaders/bs_refract.glsl" );
}


//=====================================================================================================================================
// RunStage2                                                                                                                          =
//=====================================================================================================================================
void RunStage2( const Camera& cam )
{
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w, r::h );


	glDepthMask( false );


	// render the meshes
	for( uint i=0; i<scene::meshNodes.size(); i++ )
	{
		MeshNode* mesh_node = scene::meshNodes[i];
		if( mesh_node->material->refracts )
		{
			// write to the rFbo
			intermid_fbo.Bind();
			glEnable( GL_DEPTH_TEST );
			glClear( GL_COLOR_BUFFER_BIT );
			mesh_node->material->setup();
			mesh_node->render();

			fbo.Bind();
			glDisable( GL_DEPTH_TEST );
			shader_prog->bind();
			shader_prog->locTexUnit( shader_prog->GetUniLoc(0), fai, 0 );
			r::DrawQuad( shader_prog->getAttribLoc(0) );
		}
	}


	// restore a few things
	glDepthMask( true );
	fbo_t::Unbind();
}


}} // end namespaces
