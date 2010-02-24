/**
 * The file contains functions and vars used for the deferred shading material stage.
 */

#include "renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Mesh.h"
#include "r_private.h"
#include "fbo.h"
#include "Material.h"
#include "MeshNode.h"


namespace r {
namespace ms {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static fbo_t fbo;

Texture normal_fai, diffuse_fai, specular_fai, depth_fai;



//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Init()
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(3);

	// create the FAIs
	const int internal_format = GL_RGBA16F_ARB;
	if( !normal_fai.createEmpty2D( r::w, r::h, internal_format, GL_RGBA ) ||
	    !diffuse_fai.createEmpty2D( r::w, r::h, internal_format, GL_RGBA ) ||
	    !specular_fai.createEmpty2D( r::w, r::h, internal_format, GL_RGBA ) ||
	    !depth_fai.createEmpty2D( r::w, r::h, GL_DEPTH24_STENCIL8_EXT, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT ) )
	{
		FATAL( "See prev error" );
	}

	
	// you could use the above for SSAO but the difference is very little.
	//depth_fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//depth_fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	// attach the buffers to the FBO
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, normal_fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, diffuse_fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, specular_fai.getGlId(), 0 );

	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_fai.getGlId(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create deferred shading material pass FBO" );

	// unbind
	fbo.Unbind();

#if defined( _EARLY_Z_ )
	r::ms::earlyz::init();
#endif
}


//=====================================================================================================================================
// RunStage                                                                                                                           =
//=====================================================================================================================================
void RunStage( const Camera& cam )
{
	#if defined( _EARLY_Z_ )
		// run the early z pass
		r::ms::earlyz::RunPass( cam );
	#endif

	fbo.Bind();

	#if !defined( _EARLY_Z_ )
		glClear( GL_DEPTH_BUFFER_BIT );
	#endif
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w, r::h );

	//glEnable( GL_DEPTH_TEST );
	scene::skybox.Render( cam.getViewMatrix().GetRotationPart() );
	//glDepthFunc( GL_LEQUAL );

	#if defined( _EARLY_Z_ )
		glDepthMask( false );
		glDepthFunc( GL_EQUAL );
	#endif

	// render the meshes
	for( uint i=0; i<scene::meshNodes.size(); i++ )
	{
		MeshNode* mesh_node = scene::meshNodes[i];
		DEBUG_ERR( mesh_node->material == NULL );
		if( mesh_node->material->blends || mesh_node->material->refracts ) continue;
		mesh_node->material->setup();
		mesh_node->render();
	}

	glPolygonMode( GL_FRONT, GL_FILL ); // the rendering above fucks the polygon mode


	#if defined( _EARLY_Z_ )
		glDepthMask( true );
		glDepthFunc( GL_LESS );
	#endif

	fbo.Unbind();
}


}} // end namespaces
