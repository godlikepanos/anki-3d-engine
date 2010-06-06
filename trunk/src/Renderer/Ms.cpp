#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "Camera.h"
#include "MeshNode.h"


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void Renderer::Ms::init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(3);

	// create the FAIs
	const int internal_format = GL_RGBA16F_ARB;
	if( !normalFai.createEmpty2D( r.width, r.height, internal_format, GL_RGBA ) ||
	    !diffuseFai.createEmpty2D( r.width, r.height, internal_format, GL_RGBA ) ||
	    !specularFai.createEmpty2D( r.width, r.height, internal_format, GL_RGBA ) ||
	    !depthFai.createEmpty2D( r.width, r.height, GL_DEPTH24_STENCIL8_EXT, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT ) )
	{
		FATAL( "Failed to create one MS FAI. See prev error" );
	}


	// attach the buffers to the FBO
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, normalFai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, diffuseFai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, specularFai.getGlId(), 0 );

	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depthFai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, depthFai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading material stage FBO" );

	// unbind
	fbo.unbind();

	if( earlyZ.enabled )
		earlyZ.init();
}


//=====================================================================================================================================
// run                                                                                                                                =
//=====================================================================================================================================
void Renderer::Ms::run()
{
	const Camera& cam = *r.cam;

	if( earlyZ.enabled )
	{
		earlyZ.run();
	}

	fbo.bind();

	if( !earlyZ.enabled )
	{
		glClear( GL_DEPTH_BUFFER_BIT );
	}

	r.setProjectionViewMatrices( cam );
	Renderer::setViewport( 0, 0, r.width, r.height );

	//glEnable( GL_DEPTH_TEST );
	app->getScene()->skybox.Render( cam.getViewMatrix().getRotationPart() );
	//glDepthFunc( GL_LEQUAL );

	// if earlyZ then change the default depth test and disable depth writing
	if( earlyZ.enabled )
	{
		glDepthMask( false );
		glDepthFunc( GL_EQUAL );
	}

	// render the meshes
	for( Vec<MeshNode*>::iterator it=app->getScene()->meshNodes.begin(); it!=app->getScene()->meshNodes.end(); it++ )
	{
		MeshNode* meshNode = (*it);
		DEBUG_ERR( meshNode->material == NULL );
		if( meshNode->material->blends || meshNode->material->refracts ) continue;

		r.setupMaterial( *meshNode->material, *meshNode, cam );
		meshNode->render();
	}

	glPolygonMode( GL_FRONT, GL_FILL ); // the rendering above fucks the polygon mode

	// restore depth
	if( earlyZ.enabled )
	{
		glDepthMask( true );
		glDepthFunc( GL_LESS );
	}

	fbo.unbind();
}
