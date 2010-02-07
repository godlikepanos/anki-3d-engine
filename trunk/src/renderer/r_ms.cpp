/**
 * The file contains functions and vars used for the deferred shading material stage.
 */

#include "renderer.h"
#include "camera.h"
#include "scene.h"
#include "mesh.h"
#include "r_private.h"
#include "fbo.h"
#include "material.h"
#include "mesh_node.h"


namespace r {
namespace ms {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static fbo_t fbo;

texture_t normal_fai, diffuse_fai, specular_fai, depth_fai;



//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void Init()
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(3);

	// create buffers
	const int internal_format = GL_RGBA16F_ARB;
	normal_fai.CreateEmpty2D( r::w * r::rendering_quality, r::h * r::rendering_quality, internal_format, GL_RGBA );
	diffuse_fai.CreateEmpty2D( r::w * r::rendering_quality, r::h * r::rendering_quality, internal_format, GL_RGBA );
	specular_fai.CreateEmpty2D( r::w * r::rendering_quality, r::h * r::rendering_quality, internal_format, GL_RGBA );

	//depth_fai.CreateEmpty2D( r::w * r::rendering_quality, r::h * r::rendering_quality, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT );
	depth_fai.CreateEmpty2D( r::w * r::rendering_quality, r::h * r::rendering_quality, GL_DEPTH24_STENCIL8_EXT, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT );
	// you could use the above for SSAO but the difference is very little.
	//depth_fai.TexParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//depth_fai.TexParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	// attach the buffers to the FBO
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, normal_fai.GetGLID(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, diffuse_fai.GetGLID(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, specular_fai.GetGLID(), 0 );

	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_fai.GetGLID(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_fai.GetGLID(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create deferred shading material pass FBO" );

	// unbind
	fbo.Unbind();

#if defined( _EARLY_Z_ )
	r::ms::earlyz::Init();
#endif
}


//=====================================================================================================================================
// RunStage                                                                                                                           =
//=====================================================================================================================================
void RunStage( const camera_t& cam )
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
	r::SetViewport( 0, 0, r::w * r::rendering_quality, r::h * r::rendering_quality );

	//glEnable( GL_DEPTH_TEST );
	scene::skybox.Render( cam.GetViewMatrix().GetRotationPart() );
	//glDepthFunc( GL_LEQUAL );

	#if defined( _EARLY_Z_ )
		glDepthMask( false );
		glDepthFunc( GL_EQUAL );
	#endif

	// render the meshes
	for( uint i=0; i<scene::mesh_nodes.size(); i++ )
	{
		mesh_node_t* mesh_node = scene::mesh_nodes[i];
		if( mesh_node->material->blends || mesh_node->material->refracts ) continue;
		mesh_node->material->Setup();
		mesh_node->Render();
	}

	glPolygonMode( GL_FRONT, GL_FILL ); // the rendering above fucks the polygon mode


	#if defined( _EARLY_Z_ )
		glDepthMask( true );
		glDepthFunc( GL_LESS );
	#endif

	fbo.Unbind();
}


}} // end namespaces
