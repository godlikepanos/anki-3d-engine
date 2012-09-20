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
	sms.resize(initializer.is.sm.maxLights);
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

		sm.tex.setParameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		sm.tex.setParameter(GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		sm.fbo.create();
		sm.fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, sm.tex);
	}
}

//==============================================================================
void Sm::prepareDraw()
{
	// set GL
	GlStateSingleton::get().setViewport(0, 0, resolution, resolution);

	// disable color & blend & enable depth test
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	GlStateSingleton::get().enable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	// for artifacts
	glPolygonOffset(2.0, 2.0); // keep the values as low as possible!!!!
	GlStateSingleton::get().enable(GL_POLYGON_OFFSET_FILL);
}

//==============================================================================
void Sm::afterDraw()
{
	GlStateSingleton::get().disable(GL_POLYGON_OFFSET_FILL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

//==============================================================================
void Sm::run()
{
	ANKI_ASSERT(enabled);

	Camera& cam = r->getScene().getActiveCamera();
	VisibilityInfo& vi = cam.getFrustumable()->getVisibilityInfo();

	prepareDraw();

	// Get the shadow casters
	//
	const U MAX_SHADOW_CASTERS = 256;
	ANKI_ASSERT(getMaxLightsCount() > MAX_SHADOW_CASTERS);
	Array<Light*, MAX_SHADOW_CASTERS> casters;
	U32 castersCount = 0;

	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		Light* light = (*it);

		if(light->getShadowEnabled())
		{
			casters[castersCount % getMaxLightsCount()] = light;
			++castersCount;
		}
	}

#if 1
	if(castersCount > getMaxLightsCount())
	{
		ANKI_LOGW("Too many shadow casters: " << castersCount);
	}
#endif

	castersCount = castersCount % getMaxLightsCount();

	// render all
	/*for(auto it = fr->getVisibilityInfo().getRenderablesBegin();
		it != fr->getVisibilityInfo().getRenderablesEnd(); ++it)
	{
		r->getSceneDrawer().render(r->getScene().getActiveCamera(), 1, *(*it));
	}*/

	afterDraw();
}

//==============================================================================
Sm::Shadowmap& Sm::bestCandidate(Light& light)
{
	// Allready there
	for(Shadowmap& sm : sms)
	{
		if(&light == sm.light)
		{
			return sm;
		}
	}

	// Find a null
	for(Shadowmap& sm : sms)
	{
		if(sm.light == nullptr)
		{
			sm.light = &light;
			sm.timestamp = 0;
			return sm;
		}
	}

	// Find an old
	Shadowmap* sm = &sms[0];
	for(U i = 1; i < sms.size(); i++)
	{
		if(sms[i].timestamp < sm->timestamp)
		{
			sm = &sms[i];
		}
	}

	sm->light = &light;
	sm->timestamp = 0;
	return *sm;
}

//==============================================================================
void Sm::doLight(Light& light)
{
	/// XXX Set FBO
	Shadowmap& sm = bestCandidate(light);

	Frustumable* fr = light.getFrustumable();
	ANKI_ASSERT(fr != nullptr);
	Movable* mov = &light;

	U32 lightLastUpdateTimestamp = light.getMovableTimestamp();
	lightLastUpdateTimestamp = std::max(lightLastUpdateTimestamp, 
		light.getFrustumable()->getFrustumableTimestamp());

	//for()

}

} // end namespace anki
