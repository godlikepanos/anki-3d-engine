/**
 * The file contains functions and vars used for the deferred shading blending stage stage.
 * The blending stage comes after the illumination stage. All the objects that are transculent will be drawn here.
 */

#include "renderer.h"
#include "camera.h"
#include "scene.h"
#include "mesh.h"
#include "r_private.h"
#include "resource.h"
#include "fbo.h"
#include "mesh_node.h"
#include "material.h"


namespace r {
namespace bs {

//=====================================================================================================================================
// VARS                                                                                                                               =
//=====================================================================================================================================
static fbo_t b_fbo; ///< blending models FBO
static fbo_t r_fbo; ///< refracting models FBO

/*static*/ texture_t r_fai; ///< RGB for color and A for mask (0 doesnt pass, 1 pass)
static shader_prog_t* r2b_shdr;


//=====================================================================================================================================
// InitB                                                                                                                              =
//=====================================================================================================================================
static void InitB()
{
	// create FBO
	b_fbo.Create();
	b_fbo.Bind();

	// inform FBO about the color buffers
	b_fbo.SetNumOfColorAttachements(1);

	// attach the texes
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r::is::fai.GetGLID(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, r::ms::depth_fai.GetGLID(), 0 );

	// test if success
	if( !b_fbo.IsGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	b_fbo.Unbind();
}


//=====================================================================================================================================
// InitR                                                                                                                              =
//=====================================================================================================================================
static void InitR()
{
	/*uint fbo_id, tex_id, depth_tex_id, stencil_rb;

	glGenFramebuffersEXT( 1, &fbo_id );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo_id );

	// color
	glGenTextures( 1, &tex_id );
	glBindTexture( GL_TEXTURE_2D, tex_id );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, 100, 100, 0, GL_RGB, GL_FLOAT, NULL );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0 );

	// depth
	r::PrintLastError();
	glGenTextures( 1, &depth_tex_id );
	glBindTexture( GL_TEXTURE_2D, depth_tex_id );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 100, 100, 0, GL_DEPTH_STENCIL, GL_FLOAT, NULL );
	r::PrintLastError();
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, GL_TEXTURE_2D, depth_tex_id, 0 );
	r::PrintLastError();

	// stencil
	glGenRenderbuffersEXT( 1, &stencil_rb );
	glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, stencil_rb );
	glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX, 100, 100 );
	glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, stencil_rb );

	if( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT )
		FATAL( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) );*/


	/*r::PrintLastError();
	uint depth_tex_id;
	glGenTextures( 1, &depth_tex_id );
	glBindTexture( GL_TEXTURE_2D, depth_tex_id );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8_EXT, 100, 100, 0, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, NULL );
	r::PrintLastError();*/

	// create FBO
	r_fbo.Create();
	r_fbo.Bind();

	// texture
	r_fbo.SetNumOfColorAttachements(1);
	r_fai.CreateEmpty2D( r::w * r::rendering_quality, r::h * r::rendering_quality, GL_RGBA8, GL_RGBA );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_fai.GetGLID(), 0 );

	// attach the texes
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, r::ms::depth_fai.GetGLID(), 0 );

	// test if success
	if( !r_fbo.IsGood() )
		FATAL( "Cannot create deferred shading blending stage FBO" );

	// unbind
	r_fbo.Unbind();

	r2b_shdr = rsrc::shaders.Load( "shaders/bs_refract.glsl" );
}


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void Init()
{
	InitB();
	InitR();
}


//=====================================================================================================================================
// RunStage                                                                                                                           =
//=====================================================================================================================================
void RunStage( const camera_t& cam )
{
	// OGL stuff
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w*r::rendering_quality, r::h*r::rendering_quality );

	glEnable( GL_DEPTH_TEST );
	glDepthMask( false );



	// render the meshes
	for( uint i=0; i<scene::mesh_nodes.size(); i++ )
	{
		mesh_node_t* mesh_node = scene::mesh_nodes[i];
		if( mesh_node->material->refracts )
		{
			// write to the rFbo
			r_fbo.Bind();
			glClear( GL_COLOR_BUFFER_BIT );
			mesh_node->material->Setup();
			mesh_node->Render();

			b_fbo.Bind();
			glDisable( GL_DEPTH_TEST );
			r2b_shdr->Bind();
			r2b_shdr->LocTexUnit( r2b_shdr->GetUniformLocation(0), r_fai, 0 );
			r::DrawQuad( r2b_shdr->GetAttributeLocation(0) );
		}
		else if( mesh_node->material->blends )
		{
			b_fbo.Bind();
			mesh_node->material->Setup();
			mesh_node->Render();
		}

	}


	// restore a few things
	glDepthMask( true );
	fbo_t::Unbind();
}


}} // end namespaces
