#include "renderer.hpp"
#include "scene.h"


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void renderer_t::material_stage_t::Init()
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(3);

	// create buffers
	const int internal_format = GL_RGBA16F_ARB;
	fais.normal.CreateEmpty( renderer.width, renderer.height, internal_format, GL_RGBA );
	fais.diffuse.CreateEmpty( renderer.width, renderer.height, internal_format, GL_RGBA );
	fais.specular.CreateEmpty( renderer.width, renderer.height, internal_format, GL_RGBA );

	fais.depth.CreateEmpty( renderer.width, renderer.height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT );
	// you could use the above for SSAO but the difference is minimal.
	//depth_fai.TexParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//depth_fai.TexParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	// attach the buffers to the FBO
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fais.normal.GetGLID(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, fais.diffuse.GetGLID(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, fais.specular.GetGLID(), 0 );

	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, fais.depth.GetGLID(), 0 );

	// test if success
	if( !fbo.CheckStatus() )
		FATAL( "Cannot create deferred shading material pass FBO" );

	// unbind
	fbo.Unbind();
}


//=====================================================================================================================================
// Run                                                                                                                                =
//=====================================================================================================================================
void renderer_t::material_stage_t::Run() const
{
	fbo.Bind();

	glClear( GL_DEPTH_BUFFER_BIT );
	renderer.matrices.view = renderer.camera->GetViewMatrix();
	renderer.matrices.projection = renderer.camera->GetProjectionMatrix();
	renderer.SetViewport( 0, 0, renderer.width, renderer.height );

	//glEnable( GL_DEPTH_TEST );
	scene::skybox.Render( renderer.camera->GetViewMatrix().GetRotationPart() );
	//glDepthFunc( GL_LEQUAL );


	// render the meshes
	/*for( uint i=0; i<scene::meshes.size(); i++ )
		Render<mesh_t, false>( scene::meshes[i] );

	// render the smodels
	for( uint i=0; i<scene::smodels.size(); i++ )
		Render<smodel_t, false>( scene::smodels[i] );*/

	glPolygonMode( GL_FRONT, GL_FILL ); // the rendering above fucks the polygon mode


	fbo.Unbind();
}
