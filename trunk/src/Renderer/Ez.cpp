#include "Ez.h"
#include "Renderer.h"
#include "App.h"
#include "MeshNode.h"
#include "Scene.h"
#include "RendererInitializer.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Ez::init(const RendererInitializer& initializer)
{
	enabled = initializer.ms.ez.enabled;

	if(!enabled)
	{
		return;
	}

	//
	// init FBO
	//
	try
	{
		fbo.create();
		fbo.bind();

		fbo.setNumOfColorAttachements(0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, r.getMs().getDepthFai().getGlId(), 0);

		fbo.checkIfGood();

		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create EarlyZ FBO");
	}
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Ez::run()
{
	if(!enabled)
	{
		return;
	}

	fbo.bind();

	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	for(Vec<MeshNode*>::iterator it=app->getScene().meshNodes.begin(); it!=app->getScene().meshNodes.end(); it++)
	{
		MeshNode* meshNode = (*it);
		if(meshNode->mesh->material->blends)
		{
			continue;
		}

		RASSERT_THROW_EXCEPTION(meshNode->mesh->material->dpMtl.get() == NULL);

		r.setupMaterial(*meshNode->mesh->material->dpMtl, *meshNode, r.getCamera());
		meshNode->renderDepth();
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
