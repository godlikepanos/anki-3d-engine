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
	enabled = initializer.get("is.sm.enabled");

	if(!enabled)
	{
		return;
	}

	pcfEnabled = initializer.get("is.sm.pcfEnabled");
	bilinearEnabled = initializer.get("is.sm.bilinearEnabled");
	resolution = initializer.get("is.sm.resolution");

	//
	// Init the shadowmaps
	//
	if(initializer.get("is.sm.maxLights") > MAX_SHADOW_CASTERS)
	{
		throw ANKI_EXCEPTION("Too many shadow casters");
	}

	// Create shadowmaps array
	Texture::Initializer sminit;
	sminit.target = GL_TEXTURE_2D_ARRAY;
	sminit.width = resolution;
	sminit.height = resolution;
	sminit.depth = initializer.get("is.sm.maxLights");
	sminit.format = GL_DEPTH_COMPONENT;
	sminit.internalFormat = GL_DEPTH_COMPONENT16;
	sminit.type = GL_UNSIGNED_SHORT;
	sminit.mipmapsCount = 1;
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
	sms.resize(initializer.get("is.sm.maxLights"));
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
	GlStateSingleton::get().setDepthMaskEnabled(true);

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
void Sm::run(Light* shadowCasters[], U32 shadowCastersCount)
{
	ANKI_ASSERT(enabled);

	prepareDraw();

	// render all
	for(U32 i = 0; i < shadowCastersCount; i++)
	{
		Shadowmap* sm = doLight(*shadowCasters[i]);
		ANKI_ASSERT(sm != nullptr);
		(void)sm;
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

	FrustumComponent* fr = light.getFrustumComponent();
	ANKI_ASSERT(fr != nullptr);
	VisibilityTestResults& vi = fr->getVisibilityTestResults();

	//
	// Find last update
	//
	U32 lastUpdate = light.MoveComponent::getTimestamp();
	lastUpdate = std::max(lastUpdate, fr->getTimestamp());

	for(auto it = vi.getRenderablesBegin(); it != vi.getRenderablesEnd(); ++it)
	{
		SceneNode* node = (*it).node;
		FrustumComponent* bfr = node->getFrustumComponent();
		MoveComponent* bmov = node->getMoveComponent();
		SpatialComponent* sp = node->getSpatialComponent();

		if(bfr)
		{
			lastUpdate = std::max(lastUpdate, bfr->getTimestamp());
		}

		if(bmov)
		{
			lastUpdate = std::max(lastUpdate, bmov->getTimestamp());
		}

		if(sp)
		{
			lastUpdate = std::max(lastUpdate, sp->getTimestamp());
		}
	}

	Bool shouldUpdate = lastUpdate >= sm.timestamp;

	if(!shouldUpdate)
	{
		return &sm;
	}

	sm.timestamp = getGlobTimestamp();
	light.setShadowMapIndex(&sm - &sms[0]);

	//
	// Render
	//
	sm.fbo.bind();
	r->clearAfterBindingFbo(GL_DEPTH_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	//std::cout << "Shadowmap for: " << &sm << std::endl;

	for(auto it = vi.getRenderablesBegin(); it != vi.getRenderablesEnd(); ++it)
	{
		r->getSceneDrawer().render(light, RenderableDrawer::RS_MATERIAL,
			DEPTH_PASS, *(*it).node, (*it).subSpatialIndices, 
			(*it).subSpatialIndicesCount);
	}

	return &sm;
}

} // end namespace anki
