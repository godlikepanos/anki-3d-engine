#include "Ez.h"
#include "Renderer.h"
#include "App.h"
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
	GlStateMachineSingleton::getInstance().setDepthTestEnabled(true);
	GlStateMachineSingleton::getInstance().setBlendingEnabled(false);

	/// @todo Uncomment
	/*for(Vec<MeshNode*>::iterator it=app->getScene().meshNodes.begin(); it!=app->getScene().meshNodes.end(); it++)
	{
		MeshNode* meshNode = (*it);
		if(meshNode->mesh->material->renderInBlendingStage())
		{
			continue;
		}

		//RASSERT_THROW_EXCEPTION(meshNode->mesh->material->getDepthMtl() == NULL);

		r.setupMaterial(meshNode->mesh->material->getDepthMtl(), *meshNode, r.getCamera());
		meshNode->renderDepth();
	}*/

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
