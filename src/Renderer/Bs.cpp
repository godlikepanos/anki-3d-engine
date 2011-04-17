#include <boost/ptr_container/ptr_vector.hpp>
#include "Bs.h"
#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "ShaderProg.h"
#include "Model.h"
#include "ModelNode.h"
#include "Material.h"
#include "Mesh.h"


//======================================================================================================================
// createFbo                                                                                                           =
//======================================================================================================================
void Bs::createFbo()
{
	try
	{
		fbo.create();
		fbo.bind();

		fbo.setNumOfColorAttachements(1);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		                       r.getPps().getPrePassFai().getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
		                       r.getMs().getDepthFai().getGlId(), 0);

		fbo.checkIfGood();

		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Failed to create blending stage FBO");
	}
}


//======================================================================================================================
// createRefractFbo                                                                                                    =
//======================================================================================================================
void Bs::createRefractFbo()
{
	try
	{
		refractFbo.create();
		refractFbo.bind();

		refractFbo.setNumOfColorAttachements(1);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractFai.getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
		                       r.getMs().getDepthFai().getGlId(), 0);

		refractFbo.checkIfGood();

		refractFbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Failed to create blending stage refract FBO");
	}
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Bs::init(const RendererInitializer& /*initializer*/)
{
	createFbo();
	Renderer::createFai(r.getWidth(), r.getHeight(), GL_RGBA8, GL_RGBA, GL_FLOAT, refractFai);
	createRefractFbo();
	refractSProg.loadRsrc("shaders/BsRefract.glsl");
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Bs::run()
{
	/*Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());

	glDepthMask(false);

	// render the models
	Scene::Types<ModelNode>::ConstIterator it = SceneSingleton::getInstance().getModelNodes().begin();
	for(; it != SceneSingleton::getInstance().getModelNodes().end(); ++it)
	{
		const ModelNode& mn = *(*it);
		boost::ptr_vector<ModelNodePatch>::const_iterator it = mn.getModelPatchNodes().begin();
		for(; it != mn.getModelPatchNodes().end(); it++)
		{
			const ModelNodePatch& sm = *it;

			if(!sm.getCpMtl().renderInBlendingStage())
			{
				continue;
			}

			// refracts ?
			if(sm.getCpMtl().getStdUniVar(Material::SUV_PPS_PRE_PASS_FAI))
			{
				//
				// Stage 0: Render to the temp FAI
				//
				refractFbo.bind();

				glEnable(GL_STENCIL_TEST);
				glClear(GL_STENCIL_BUFFER_BIT);
				glStencilFunc(GL_ALWAYS, 0x1, 0x1);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

				r.setupShaderProg(sm.getCpMtl(), mn, r.getCamera());
				glDisable(GL_BLEND); // a hack

				sm.getCpVao().bind();
				glDrawElements(GL_TRIANGLES, sm.getModelPatchRsrc().getMesh().getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
				sm.getCpVao().unbind();

				//
				// Stage 1: Render the temp FAI to prePassFai
				//
				fbo.bind();

				glStencilFunc(GL_EQUAL, 0x1, 0x1);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

				if(sm.getCpMtl().isBlendingEnabled())
				{
					glEnable(GL_BLEND);
					glBlendFunc(sm.getCpMtl().getBlendingSfactor(), sm.getCpMtl().getBlendingDfactor());
				}
				else
				{
					glDisable(GL_BLEND);
				}

				refractSProg->bind();
				refractSProg->findUniVar("fai")->setTexture(refractFai, 0);

				r.drawQuad();

				// cleanup
				glStencilFunc(GL_ALWAYS, 0x1, 0x1);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glClear(GL_STENCIL_BUFFER_BIT);
				glDisable(GL_STENCIL_TEST);
			}
			// no rafraction
			else
			{
				r.setupShaderProg(sm.getCpMtl(), mn, r.getCamera());

				sm.getCpVao().bind();
				glDrawElements(GL_TRIANGLES, sm.getModelPatchRsrc().getMesh().getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
				sm.getCpVao().unbind();
			}
		} // end for all subModels
	} // end for all modelNodes

	glDepthMask(true);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // the rendering above fucks the polygon mode
	Fbo::unbind();*/
}
