#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "Camera.h"
#include "MeshNode.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::Ms::init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(3);

	// create the FAIs
	if(!normalFai.createEmpty2D(r.width, r.height, GL_RG16F, GL_RG, GL_FLOAT) ||
	   !diffuseFai.createEmpty2D(r.width, r.height, GL_RGB16F, GL_RGB, GL_FLOAT) ||
	   !specularFai.createEmpty2D(r.width, r.height, GL_RGBA16F, GL_RGBA, GL_FLOAT) ||
	   !depthFai.createEmpty2D(r.width, r.height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8))
	   //!depthFai.createEmpty2D(r.width, r.height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, false))
	{
		FATAL("Failed to create one MS FAI. See prev error");
	}

	normalFai.setRepeat(false);
	diffuseFai.setRepeat(false);
	specularFai.setRepeat(false);
	depthFai.setRepeat(true);


	// attach the buffers to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalFai.getGlId(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, diffuseFai.getGlId(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, specularFai.getGlId(), 0);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthFai.getGlId(), 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthFai.getGlId(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthFai.getGlId(), 0);

	// test if success
	if(!fbo.isGood())
		FATAL("Cannot create deferred shading material stage FBO");

	// unbind
	fbo.unbind();

	if(ez.enabled)
		ez.init();
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Renderer::Ms::run()
{
	const Camera& cam = *r.cam;

	if(ez.enabled)
	{
		ez.run();
	}

	fbo.bind();

	if(!ez.enabled)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	r.setProjectionViewMatrices(cam); ///< @todo remove this
	Renderer::setViewport(0, 0, r.width, r.height);

	//glEnable(GL_DEPTH_TEST);
	app->getScene().skybox.Render(cam.getViewMatrix().getRotationPart());
	//glDepthFunc(GL_LEQUAL);

	// if ez then change the default depth test and disable depth writing
	if(ez.enabled)
	{
		glDepthMask(false);
		glDepthFunc(GL_EQUAL);
	}

	// render the meshes
	for(Vec<MeshNode*>::iterator it=app->getScene().meshNodes.begin(); it!=app->getScene().meshNodes.end(); it++)
	{
		MeshNode* meshNode = (*it);
		if(meshNode->mesh->material.get() == NULL)
		{
			ERROR("Mesh \"" << meshNode->mesh->getRsrcName() << "\" doesnt have material" );
			continue;
		}
		if(meshNode->mesh->material->blends) continue;

		r.setupMaterial(*meshNode->mesh->material, *meshNode, cam);
		meshNode->render();
	}

	glPolygonMode(GL_FRONT, GL_FILL); // the rendering above fucks the polygon mode

	// restore depth
	if(ez.enabled)
	{
		glDepthMask(true);
		glDepthFunc(GL_LESS);
	}

	fbo.unbind();
}
