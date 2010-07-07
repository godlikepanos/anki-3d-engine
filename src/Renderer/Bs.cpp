#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "MeshNode.h"


//======================================================================================================================
// createFbo                                                                                                           =
//======================================================================================================================
void Renderer::Bs::createFbo()
{
	fbo.create();
	fbo.bind();

	fbo.setNumOfColorAttachements(1);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r.pps.prePassFai.getGlId(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0);

	if(!fbo.isGood())
		FATAL("Cannot create deferred shading blending stage FBO");

	fbo.unbind();
}


//======================================================================================================================
// createRefractFbo                                                                                                    =
//======================================================================================================================
void Renderer::Bs::createRefractFbo()
{
	refractFbo.create();
	refractFbo.bind();

	refractFbo.setNumOfColorAttachements(1);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractFai.getGlId(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0);

	if(!refractFbo.isGood())
		FATAL("Cannot create deferred shading blending stage FBO");

	refractFbo.unbind();
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::Bs::init()
{
	createFbo();
	refractFai.createEmpty2D(r.width, r.height, GL_RGBA8, GL_RGBA, GL_FLOAT, false);
	createRefractFbo();

	refractSProg.reset(Resource::shaders.load("shaders/BsRefract.glsl"));
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Renderer::Bs::run()
{
	Renderer::setViewport(0, 0, r.width, r.height);

	glDepthMask(false);

	// render the meshes
	for(Vec<MeshNode*>::iterator it=app->getScene()->meshNodes.begin(); it!=app->getScene()->meshNodes.end(); it++)
	{
		MeshNode* meshNode = (*it);
		DEBUG_ERR(meshNode->material == NULL);
		if(!meshNode->material->blends) continue;

		// refracts
		if(meshNode->material->stdUniVars[Material::SUV_PPS_PRE_PASS_FAI])
		{
			// render to the temp FAI
			refractFbo.bind();

			glEnable(GL_STENCIL_TEST);
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilFunc(GL_ALWAYS, 0x1, 0x1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			r.setupMaterial(*meshNode->material, *meshNode, *r.cam);
			glDisable(GL_BLEND); // a hack
			meshNode->render();

			// render the temp FAI to prePassFai
			fbo.bind();

			glStencilFunc(GL_EQUAL, 0x1, 0x1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			if(meshNode->material->blends)
			{
				glEnable(GL_BLEND);
				glBlendFunc(meshNode->material->blendingSfactor, meshNode->material->blendingDfactor);
			}
			else
				glDisable(GL_BLEND);

			refractSProg->bind();
			refractSProg->findUniVar("fai")->setTexture(refractFai, 0);

			Renderer::drawQuad(0);

			// cleanup
			glStencilFunc(GL_ALWAYS, 0x1, 0x1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glClear(GL_STENCIL_BUFFER_BIT);
			glDisable(GL_STENCIL_TEST);
		}
		else
		{
			fbo.bind();
			r.setupMaterial(*meshNode->material, *meshNode, *r.cam);
			meshNode->render();
		}
	}

	glDepthMask(true);
	glPolygonMode(GL_FRONT, GL_FILL); // the rendering above fucks the polygon mode
	Fbo::unbind();
}
