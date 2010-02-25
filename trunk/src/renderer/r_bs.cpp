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
static fbo_t fbo; ///< blending models FBO


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Init()
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform FBO about the color buffers
	fbo.SetNumOfColorAttachements(1);

	// attach the texes
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r::is::fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, r::ms::depth_fai.getGlId(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	fbo.Unbind();
}


//=====================================================================================================================================
// RunStage                                                                                                                           =
//=====================================================================================================================================
void RunStage( const Camera& cam )
{
	// OGL stuff
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w, r::h );

	glEnable( GL_DEPTH_TEST );
	glDepthMask( false );



	// render the meshes
	for( uint i=0; i<Scene::meshNodes.size(); i++ )
	{
		MeshNode* mesh_node = Scene::meshNodes[i];
		if( mesh_node->material->blends && !mesh_node->material->blends )
		{
			fbo.Bind();
			mesh_node->material->setup();
			mesh_node->render();
		}

	}


	// restore a few things
	glDepthMask( true );
	fbo_t::Unbind();
}


}} // end namespaces
