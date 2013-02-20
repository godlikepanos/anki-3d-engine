#include "anki/renderer/Sm.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/SceneGraph.h"
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

	//
	// Init the shadowmaps
	//
	if(initializer.is.sm.maxLights > MAX_SHADOW_CASTERS)
	{
		throw ANKI_EXCEPTION("Too many shadow casters");
	}

	// Create shadowmaps array
	Texture::Initializer sminit;
	sminit.target = GL_TEXTURE_2D_ARRAY;
	sminit.width = resolution;
	sminit.height = resolution;
	sminit.depth = initializer.is.sm.maxLights;
	sminit.format = GL_DEPTH_COMPONENT;
	sminit.internalFormat = GL_DEPTH_COMPONENT16;
	sminit.type = GL_FLOAT;
	if(bilinearEnabled)
	{
		sminit.filteringType = Texture::TFT_LINEAR;
	}
	else
	{
		sminit.filteringType = Texture::TFT_NEAREST;
	}
	sm2DArrayTex.create(sminit);

	sm2DArrayTex.setParameter(GL_TEXTURE_COMPARE_MODE, 
		GL_COMPARE_REF_TO_TEXTURE);
	sm2DArrayTex.setParameter(GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// Init sms
	U32 layer = 0;
	sms.resize(initializer.is.sm.maxLights);
	for(Shadowmap& sm : sms)
	{
		sm.layerId = layer;
		sm.fbo.create();
		sm.fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, sm2DArrayTex, layer, -1);

		++layer;
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

	r->getSceneDrawer().prepareDraw();
}

//==============================================================================
void Sm::afterDraw()
{
	GlStateSingleton::get().disable(GL_POLYGON_OFFSET_FILL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

//==============================================================================
void Sm::run(Light* shadowCasters[], U32 shadowCastersCount, 
	Array<U32,  MAX_SHADOW_CASTERS>& shadowmapLayers)
{
	ANKI_ASSERT(enabled);

	prepareDraw();

	// render all
	for(U32 i = 0; i < shadowCastersCount; i++)
	{
		Shadowmap* sm = doLight(*shadowCasters[i]);
		ANKI_ASSERT(sm != nullptr);
		shadowmapLayers[i] = sm->layerId;
	}

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

	// Find an old and replace it
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
Sm::Shadowmap* Sm::doLight(Light& light)
{
	Shadowmap& sm = bestCandidate(light);

	Frustumable* fr = light.getFrustumable();
	ANKI_ASSERT(fr != nullptr);
	VisibilityTestResults& vi = *fr->getVisibilityTestResults();

	//
	// Find last update
	//
	U32 lastUpdate = light.getMovableTimestamp();
	lastUpdate = std::max(lastUpdate, fr->getFrustumableTimestamp());

	for(auto it = vi.getRenderablesBegin(); it != vi.getRenderablesEnd(); ++it)
	{
		SceneNode* node = *it;
		Frustumable* bfr = node->getFrustumable();
		Movable* bmov = node->getMovable();
		Spatial* sp = node->getSpatial();

		if(bfr)
		{
			lastUpdate = std::max(lastUpdate, bfr->getFrustumableTimestamp());
		}

		if(bmov)
		{
			lastUpdate = std::max(lastUpdate, bmov->getMovableTimestamp());
		}

		if(sp)
		{
			lastUpdate = std::max(lastUpdate, sp->getSpatialTimestamp());
		}
	}

	Bool shouldUpdate = lastUpdate >= sm.timestamp;

	if(!shouldUpdate)
	{
		return &sm;
	}

	sm.timestamp = Timestamp::getTimestamp();

	//
	// Render
	//
	sm.fbo.bind();
	r->clearAfterBindingFbo(GL_DEPTH_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	//std::cout << "Shadowmap for: " << &sm << std::endl;

	for(auto it = vi.getRenderablesBegin(); it != vi.getRenderablesEnd(); ++it)
	{
		r->getSceneDrawer().render(*fr, RenderableDrawer::RS_MATERIAL,
			1, *(*it));
	}

	return &sm;
}

} // end namespace anki
