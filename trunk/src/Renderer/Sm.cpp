#include "Sm.h"
#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "MeshNode.h"
#include "LightData.h"
#include "Camera.h"
#include "RendererInitializer.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Sm::init(const RendererInitializer& initializer)
{
	enabled = initializer.is.sm.enabled;

	if(!enabled)
		return;

	pcfEnabled = initializer.is.sm.pcfEnabled;
	bilinearEnabled = initializer.is.sm.bilinearEnabled;
	resolution = initializer.is.sm.resolution;

	try
	{
		// create FBO
		fbo.create();
		fbo.bind();

		// texture
		shadowMap.createEmpty2D(resolution, resolution, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
		if(bilinearEnabled)
		{
			shadowMap.setFiltering(Texture::TFT_LINEAR);
		}
		else
		{
			shadowMap.setFiltering(Texture::TFT_NEAREST);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		/*
		 * If you dont want to use the FFP for comparing the shadowmap (the above two lines) then you can make the comparison
		 * inside the glsl shader. The GL_LEQUAL means that: shadow = (R <= Dt) ? 1.0 : 0.0; . The R is given by:
		 * R = _tex_coord2.z/_tex_coord2.w; and the Dt = shadow2D(shadow_depth_map, _shadow_uv).r (see lp_generic.frag).
		 * Hardware filters like GL_LINEAR cannot be applied.
		 */

		// inform the we wont write to color buffers
		fbo.setNumOfColorAttachements(0);

		// attach the texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap.getGlId(), 0);

		// test if success
		fbo.checkIfGood();

		// unbind
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create shadowmapping FBO: " + e.what());
	}
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void Sm::run(const Camera& cam)
{
	if(!enabled)
	{
		return;
	}

	// FBO
	fbo.bind();

	// set GL
	Renderer::setViewport(0, 0, resolution, resolution);
	glClear(GL_DEPTH_BUFFER_BIT);

	// disable color & blend & enable depth test
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// for artifacts
	glPolygonOffset(2.0, 2.0); // keep the values as low as possible!!!!
	glEnable(GL_POLYGON_OFFSET_FILL);

	// render all meshes
	for(Vec<MeshNode*>::iterator it=app->getScene().meshNodes.begin(); it!=app->getScene().meshNodes.end(); it++)
	{
		MeshNode* meshNode = (*it);
		if(meshNode->mesh->material->blends)
		{
			continue;
		}

		RASSERT_THROW_EXCEPTION(meshNode->mesh->material->dpMtl.get() == NULL);

		r.setupMaterial(*meshNode->mesh->material->dpMtl, *meshNode, cam);
		meshNode->renderDepth();
	}

	// restore GL
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	// FBO
	fbo.unbind();
}
