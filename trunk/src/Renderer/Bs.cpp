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

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r.pps.prePassFai.getGlId(), 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0);

	if(!fbo.isGood()) FATAL("Cannot create deferred shading blending stage FBO");

	fbo.unbind();
}


//======================================================================================================================
// createRefractFbo                                                                                                    =
//======================================================================================================================
void Renderer::Bs::createRefractFbo()
{
	fbo.create();
	fbo.bind();

	fbo.setNumOfColorAttachements(1);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, refractFai.getGlId(), 0);

	if(!fbo.isGood()) FATAL("Cannot create deferred shading blending stage FBO");

	fbo.unbind();
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::Bs::init()
{
	createFbo();
	createRefractFbo();
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Renderer::Bs::run()
{
	Renderer::setViewport(0, 0, r.width, r.height);


	// render the meshes
	for(Vec<MeshNode*>::iterator it=app->getScene()->meshNodes.begin(); it!=app->getScene()->meshNodes.end(); it++)
	{
		MeshNode* meshNode = (*it);
		DEBUG_ERR(meshNode->material == NULL);
		if(!meshNode->material->blends) continue;

		r.setupMaterial(*meshNode->material, *meshNode, *r.cam);

		// refracts
		if(meshNode->material->stdUniVars[Material::SUV_PPS_PRE_PASS_FAI] ||
		   meshNode->material->stdUniVars[Material::SUV_PPS_POST_PASS_FAI])
		{

		}
		else
		{

		}
	}

	Fbo::unbind();
	glPolygonMode(GL_FRONT, GL_FILL); // the rendering above fucks the polygon mode
}
