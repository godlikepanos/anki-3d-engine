// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Sm.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/core/Counters.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
Error Sm::init(const ConfigSet& initializer)
{
	m_enabled = initializer.get("is.sm.enabled");

	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	m_poissonEnabled = initializer.get("is.sm.poissonEnabled");
	m_bilinearEnabled = initializer.get("is.sm.bilinearEnabled");
	m_resolution = initializer.get("is.sm.resolution");

	//
	// Init the shadowmaps
	//
	if(initializer.get("is.sm.maxLights") > MAX_SHADOW_CASTERS)
	{
		ANKI_LOGE("Too many shadow casters");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Create shadowmaps array
	TextureHandle::Initializer sminit;
	sminit.m_target = GL_TEXTURE_2D_ARRAY;
	sminit.m_width = m_resolution;
	sminit.m_height = m_resolution;
	sminit.m_depth = initializer.get("is.sm.maxLights");
	sminit.m_internalFormat = GL_DEPTH_COMPONENT16;
	sminit.m_mipmapsCount = 1;
	if(m_bilinearEnabled)
	{
		sminit.m_filterType = TextureHandle::Filter::LINEAR;
	}
	else
	{
		sminit.m_filterType = TextureHandle::Filter::NEAREST;
	}

	CommandBufferHandle cmdBuff;
	ANKI_CHECK(cmdBuff.create(&getGrManager()));

	ANKI_CHECK(m_sm2DArrayTex.create(cmdBuff, sminit));

	m_sm2DArrayTex.setParameter(cmdBuff, 
		GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	m_sm2DArrayTex.setParameter(cmdBuff, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// Init sms
	U32 layer = 0;
	m_sms.create(getAllocator(), initializer.get("is.sm.maxLights"));
	for(Shadowmap& sm : m_sms)
	{
		sm.m_layerId = layer;

		FramebufferHandle::Initializer fbInit;
		fbInit.m_depthStencilAttachment.m_texture = m_sm2DArrayTex;
		fbInit.m_depthStencilAttachment.m_layer = layer;
		ANKI_CHECK(sm.m_fb.create(cmdBuff, fbInit));

		++layer;
	}

	cmdBuff.flush();

	return ErrorCode::NONE;
}

//==============================================================================
void Sm::prepareDraw(CommandBufferHandle& cmdBuff)
{
	// disable color & blend & enable depth test

	cmdBuff.enableDepthTest(true);
	cmdBuff.setColorWriteMask(false, false, false, false);

	// for artifacts
	cmdBuff.setPolygonOffset(7.0, 5.0); // keep both as low as possible!!!!
	cmdBuff.enablePolygonOffset(true);

	m_r->getSceneDrawer().prepareDraw(
		RenderingStage::MATERIAL, Pass::DEPTH, cmdBuff);
}

//==============================================================================
void Sm::finishDraw(CommandBufferHandle& cmdBuff)
{
	m_r->getSceneDrawer().finishDraw();

	cmdBuff.enableDepthTest(false);
	cmdBuff.enablePolygonOffset(false);
	cmdBuff.setColorWriteMask(true, true, true, true);
}

//==============================================================================
Error Sm::run(SceneNode* shadowCasters[], U32 shadowCastersCount, 
	CommandBufferHandle& cmdBuff)
{
	ANKI_ASSERT(m_enabled);
	Error err = ErrorCode::NONE;

	prepareDraw(cmdBuff);

	// render all
	for(U32 i = 0; i < shadowCastersCount; i++)
	{
		Shadowmap* sm;
		err = doLight(*shadowCasters[i], cmdBuff, sm);
		if(err) return err;

		ANKI_ASSERT(sm != nullptr);
		(void)sm;
	}

	finishDraw(cmdBuff);

	return err;
}

//==============================================================================
Sm::Shadowmap& Sm::bestCandidate(SceneNode& light)
{
	// Allready there
	for(Shadowmap& sm : m_sms)
	{
		if(&light == sm.m_light)
		{
			return sm;
		}
	}

	// Find a null
	for(Shadowmap& sm : m_sms)
	{
		if(sm.m_light == nullptr)
		{
			sm.m_light = &light;
			sm.m_timestamp = 0;
			return sm;
		}
	}

	// Find an old and replace it
	Shadowmap* sm = &m_sms[0];
	for(U i = 1; i < m_sms.getSize(); i++)
	{
		if(m_sms[i].m_timestamp < sm->m_timestamp)
		{
			sm = &m_sms[i];
		}
	}

	sm->m_light = &light;
	sm->m_timestamp = 0;
	return *sm;
}

//==============================================================================
Error Sm::doLight(
	SceneNode& light, CommandBufferHandle& cmdBuff, Sm::Shadowmap*& sm)
{
	Error err = ErrorCode::NONE;

	sm = &bestCandidate(light);

	FrustumComponent& fr = light.getComponent<FrustumComponent>();
	VisibilityTestResults& vi = fr.getVisibilityTestResults();
	LightComponent& lcomp = light.getComponent<LightComponent>();

	//
	// Find last update
	//
	U32 lastUpdate = light.getComponent<MoveComponent>().getTimestamp();
	lastUpdate = std::max(lastUpdate, fr.getTimestamp());

	auto it = vi.getRenderablesBegin();
	auto end = vi.getRenderablesEnd();
	for(; it != end; ++it)
	{
		SceneNode* node = it->m_node;

		FrustumComponent* bfr = node->tryGetComponent<FrustumComponent>();
		if(bfr)
		{
			lastUpdate = std::max(lastUpdate, bfr->getTimestamp());
		}

		MoveComponent* bmov = node->tryGetComponent<MoveComponent>();
		if(bmov)
		{
			lastUpdate = std::max(lastUpdate, bmov->getTimestamp());
		}

		SpatialComponent* sp = node->tryGetComponent<SpatialComponent>();
		if(sp)
		{
			lastUpdate = std::max(lastUpdate, sp->getTimestamp());
		}
	}

	Bool shouldUpdate = lastUpdate >= sm->m_timestamp;
	if(!shouldUpdate)
	{
		return err;
	}

	sm->m_timestamp = getGlobalTimestamp();
	lcomp.setShadowMapIndex(sm - &m_sms[0]);

	//
	// Render
	//
	sm->m_fb.bind(cmdBuff, true);
	cmdBuff.setViewport(0, 0, m_resolution, m_resolution);
	cmdBuff.clearBuffers(GL_DEPTH_BUFFER_BIT);

	it = vi.getRenderablesBegin();
	for(; it != end; ++it)
	{
		err = m_r->getSceneDrawer().render(light, *it);
		if(err) break;
	}

	ANKI_COUNTER_INC(RENDERER_SHADOW_PASSES, (U64)1);

	return err;
}

} // end namespace anki
