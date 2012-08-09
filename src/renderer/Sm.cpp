#include "anki/renderer/Sm.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
void Sm::init(const RendererInitializer& initializer)
{
	enabled = initializer.is.sm.enabled;

	if(!enabled)
	{
		return;
	}

	pcfEnabled = initializer.is.sm.pcfEnabled;
	bilinearEnabled = initializer.is.sm.bilinearEnabled;
	resolution = initializer.is.sm.resolution;

	// Init the shadowmaps
	for(Shadowmap& sm : sms)
	{
		Renderer::createFai(resolution, resolution,
			GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, sm.tex);

		if(bilinearEnabled)
		{
			sm.tex.setFiltering(Texture::TFT_LINEAR);
		}
		else
		{
			sm.tex.setFiltering(Texture::TFT_NEAREST);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		sm.fbo.create();
		sm.fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, sm.tex);
	}
}

//==============================================================================
Sm::Shadowmap& Sm::findBestCandidate(Light& light)
{
	uint32_t crntFrame = r->getFramesCount();
	uint32_t minFrame = crntFrame;
	Shadowmap* best = nullptr;
	Shadowmap* secondBest = nullptr;

	// Check if already in list
	for(Shadowmap& sm : sms)
	{
		uint32_t fr;

		if(sm.light == &light)
		{
			return sm;
		}
		else if(sm.light == nullptr)
		{
			best = &sm;
		}
		else if((fr = sm.light->getLastUpdateFrame()) < minFrame)
		{
			secondBest = &sm;
			minFrame = fr;
		}
	}

	ANKI_ASSERT(best != nullptr || secondBest != nullptr);

	return (best) ? *best : *secondBest;
}

//==============================================================================
Texture* Sm::run(Light& light, float distance)
{
	if(!enabled || !light.getShadowEnabled()
		|| light.getLightType() == Light::LT_POINT)
	{
		return nullptr;
	}

	Shadowmap& sm = findBestCandidate(light);

	// Render
	//

	// FBO
	sm.fbo.bind();

	// set GL
	GlStateSingleton::get().setViewport(0, 0, resolution, resolution);
	glClear(GL_DEPTH_BUFFER_BIT);

	// disable color & blend & enable depth test
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	GlStateSingleton::get().enable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	// for artifacts
	glPolygonOffset(2.0, 2.0); // keep the values as low as possible!!!!
	GlStateSingleton::get().enable(GL_POLYGON_OFFSET_FILL);

	Frustumable* fr = light.getFrustumable();
	ANKI_ASSERT(fr);

	// render all
	for(auto it = fr->getVisibilityInfo().getRenderablesBegin();
		it != fr->getVisibilityInfo().getRenderablesEnd(); ++it)
	{
		r->getSceneDrawer().render(r->getScene().getActiveCamera(), 1, *(*it));
	}

	// restore GL
	GlStateSingleton::get().disable(GL_POLYGON_OFFSET_FILL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	return &sm.tex;
}


} // end namespace
