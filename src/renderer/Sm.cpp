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
	TexturePtr::Initializer sminit;
	sminit.m_type = TextureType::_2D_ARRAY;
	sminit.m_width = m_resolution;
	sminit.m_height = m_resolution;
	sminit.m_depth = initializer.get("is.sm.maxLights");
	sminit.m_format = PixelFormat(ComponentFormat::D16, TransformFormat::FLOAT);
	sminit.m_mipmapsCount = 1;
	sminit.m_sampling.m_minMagFilter = m_bilinearEnabled 
		? SamplingFilter::LINEAR 
		: SamplingFilter::NEAREST;
	sminit.m_sampling.m_compareOperation = CompareOperation::LESS_EQUAL;

	CommandBufferPtr cmdBuff;
	ANKI_CHECK(cmdBuff.create(&getGrManager()));

	ANKI_CHECK(m_sm2DArrayTex.create(cmdBuff, sminit));
	cmdBuff.flush();

	// Init sms
	m_sms.create(getAllocator(), initializer.get("is.sm.maxLights"));

	FramebufferPtr::Initializer fbInit;
	fbInit.m_depthStencilAttachment.m_texture = m_sm2DArrayTex;
	fbInit.m_depthStencilAttachment.m_loadOperation = 
		AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

	U32 layer = 0;
	for(Shadowmap& sm : m_sms)
	{
		sm.m_layerId = layer;

		fbInit.m_depthStencilAttachment.m_layer = layer;
		ANKI_CHECK(sm.m_fb.create(&getGrManager(), fbInit));

		++layer;
	}

	return ErrorCode::NONE;
}

//==============================================================================
void Sm::prepareDraw(CommandBufferPtr& cmdBuff)
{
	// disable color & blend & enable depth test

	cmdBuff.enableDepthTest(true);
	cmdBuff.setDepthWriteMask(true);
	cmdBuff.setColorWriteMask(false, false, false, false);

	// for artifacts
	cmdBuff.setPolygonOffset(7.0, 5.0); // keep both as low as possible!!!!
	cmdBuff.enablePolygonOffset(true);

	m_r->getSceneDrawer().prepareDraw(
		RenderingStage::MATERIAL, Pass::DEPTH, cmdBuff);
}

//==============================================================================
void Sm::finishDraw(CommandBufferPtr& cmdBuff)
{
	m_r->getSceneDrawer().finishDraw();

	cmdBuff.enableDepthTest(false);
	cmdBuff.setDepthWriteMask(false);
	cmdBuff.enablePolygonOffset(false);
	cmdBuff.setColorWriteMask(true, true, true, true);
}

//==============================================================================
Error Sm::run(SceneNode* shadowCasters[], U32 shadowCastersCount, 
	CommandBufferPtr& cmdBuff)
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
	SceneNode& light, CommandBufferPtr& cmdBuff, Sm::Shadowmap*& sm)
{
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
		return ErrorCode::NONE;
	}

	sm->m_timestamp = getGlobalTimestamp();
	lcomp.setShadowMapIndex(sm - &m_sms[0]);

	//
	// Render
	//
	sm->m_fb.bind(cmdBuff);
	cmdBuff.setViewport(0, 0, m_resolution, m_resolution);

	it = vi.getRenderablesBegin();
	for(; it != end; ++it)
	{
		ANKI_CHECK(m_r->getSceneDrawer().render(light, *it));
	}

	ANKI_COUNTER_INC(RENDERER_SHADOW_PASSES, (U64)1);

	return ErrorCode::NONE;
}

} // end namespace anki
