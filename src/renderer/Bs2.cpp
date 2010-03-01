/**
 * The file contains functions and vars used for the deferred shading blending stage stage.
 * The blending stage comes after the illumination stage. All the objects that are transculent will be drawn here.
 */

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Mesh.h"
#include "Resource.h"
#include "Fbo.h"
#include "MeshNode.h"
#include "Material.h"


namespace R {
namespace Bs {

//=====================================================================================================================================
// VARS                                                                                                                               =
//=====================================================================================================================================
static Fbo intermid_fbo, fbo;

static Texture fai; ///< RGB for color and A for mask (0 doesnt pass, 1 pass)
static ShaderProg* shader_prog;


//=====================================================================================================================================
// init2                                                                                                                              =
//=====================================================================================================================================
void init2()
{
	//** 1st FBO **
	// create FBO
	fbo.Create();
	fbo.bind();

	// inform FBO about the color buffers
	fbo.setNumOfColorAttachements(1);

	// attach the texes
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, R::Pps::fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, R::Ms::depthFai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	fbo.Unbind();


	//** 2nd FBO **
	intermid_fbo.Create();
	intermid_fbo.bind();

	// texture
	intermid_fbo.setNumOfColorAttachements(1);
	fai.createEmpty2D( R::w, R::h, GL_RGBA8, GL_RGBA );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// attach the texes
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, R::Ms::depthFai.getGlId(), 0 );

	// test if success
	if( !intermid_fbo.isGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	intermid_fbo.Unbind();

	shader_prog = rsrc::shaders.load( "shaders/bs_refract.glsl" );
}


//=====================================================================================================================================
// runStage2                                                                                                                          =
//=====================================================================================================================================
void runStage2( const Camera& cam )
{
	R::setProjectionViewMatrices( cam );
	R::setViewport( 0, 0, R::w, R::h );


	glDepthMask( false );


	// render the meshes
	for( uint i=0; i<Scene::meshNodes.size(); i++ )
	{
		MeshNode* mesh_node = Scene::meshNodes[i];
		if( mesh_node->material->refracts )
		{
			// write to the rFbo
			intermid_fbo.bind();
			glEnable( GL_DEPTH_TEST );
			glClear( GL_COLOR_BUFFER_BIT );
			mesh_node->material->setup();
			mesh_node->render();

			fbo.bind();
			glDisable( GL_DEPTH_TEST );
			shader_prog->bind();
			shader_prog->locTexUnit( shader_prog->getUniLoc(0), fai, 0 );
			R::DrawQuad( shader_prog->getAttribLoc(0) );
		}
	}


	// restore a few things
	glDepthMask( true );
	Fbo::Unbind();
}


}} // end namespaces
