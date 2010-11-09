#include "Ms.h"
#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "Camera.h"
#include "MeshNode.h"
#include "Ez.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Ms::Ms(Renderer& r_, Object* parent):
	RenderingPass(r_, parent),
	ez(new Ez(r_, this))
{}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Ms::init(const RendererInitializer& initializer)
{
	try
	{
		// create FBO
		fbo.create();
		fbo.bind();

		// inform in what buffers we draw
		fbo.setNumOfColorAttachements(3);

		// create the FAIs
		normalFai.createEmpty2D(r.getWidth(), r.getHeight(), GL_RG16F, GL_RG, GL_FLOAT);
		diffuseFai.createEmpty2D(r.getWidth(), r.getHeight(), GL_RGB16F, GL_RGB, GL_FLOAT);
		specularFai.createEmpty2D(r.getWidth(), r.getHeight(), GL_RGBA16F, GL_RGBA, GL_FLOAT);
		depthFai.createEmpty2D(r.getWidth(), r.getHeight(), GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);

		normalFai.setRepeat(false);
		diffuseFai.setRepeat(false);
		specularFai.setRepeat(false);
		depthFai.setRepeat(false);

		// attach the buffers to the FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalFai.getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, diffuseFai.getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, specularFai.getGlId(), 0);

		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthFai.getGlId(), 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthFai.getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthFai.getGlId(), 0);

		// test if success
		fbo.checkIfGood();

		// unbind
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create deferred shading material stage FBO: " + e.what());
	}

	ez->init(initializer);
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Ms::run()
{
	const Camera& cam = r.getCamera();

	if(ez->isEnabled())
	{
		ez->run();
	}

	fbo.bind();

	if(!ez->isEnabled())
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	//glEnable(GL_DEPTH_TEST);
	//app->getScene().skybox.Render(cam.getViewMatrix().getRotationPart());
	//glDepthFunc(GL_LEQUAL);

	// if ez then change the default depth test and disable depth writing
	if(ez->isEnabled())
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
			throw EXCEPTION("Mesh \"" + meshNode->mesh->getRsrcName() + "\" doesnt have material");
		}

		if(meshNode->mesh->material->blends)
		{
			continue;
		}

		r.setupMaterial(*meshNode->mesh->material, *meshNode, cam);
		meshNode->render();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // the rendering above fucks the polygon mode

	// restore depth
	if(ez->isEnabled())
	{
		glDepthMask(true);
		glDepthFunc(GL_LESS);
	}

	fbo.unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}
