// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Sm.h>
#include <anki/renderer/Renderer.h>
#include <anki/core/App.h>
#include <anki/core/Trace.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Camera.h>
#include <anki/scene/Light.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

//==============================================================================
const PixelFormat Sm::DEPTH_RT_PIXEL_FORMAT(
	ComponentFormat::D16, TransformFormat::FLOAT);

//==============================================================================
Error Sm::init(const ConfigSet& config)
{
	m_enabled = config.getNumber("is.sm.enabled");

	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	m_poissonEnabled = config.getNumber("is.sm.poissonEnabled");
	m_bilinearEnabled = config.getNumber("is.sm.bilinearEnabled");
	m_resolution = config.getNumber("is.sm.resolution");

	//
	// Init the shadowmaps
	//
	if(config.getNumber("is.sm.maxLights") > MAX_SHADOW_CASTERS)
	{
		ANKI_LOGE("Too many shadow casters");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Create shadowmaps array
	TextureInitializer sminit;
	sminit.m_type = TextureType::_2D_ARRAY;
	sminit.m_width = m_resolution;
	sminit.m_height = m_resolution;
	sminit.m_depth = config.getNumber("is.sm.maxLights");
	sminit.m_format = DEPTH_RT_PIXEL_FORMAT;
	sminit.m_mipmapsCount = 1;
	sminit.m_sampling.m_minMagFilter =
		m_bilinearEnabled ? SamplingFilter::LINEAR : SamplingFilter::NEAREST;
	sminit.m_sampling.m_compareOperation = CompareOperation::LESS_EQUAL;

	m_spotTexArray = getGrManager().newInstance<Texture>(sminit);

	sminit.m_type = TextureType::CUBE_ARRAY;
	m_omniTexArray = getGrManager().newInstance<Texture>(sminit);

	// Init 2D layers
	m_spots.create(getAllocator(), config.getNumber("is.sm.maxLights"));

	FramebufferInitializer fbInit;
	fbInit.m_depthStencilAttachment.m_texture = m_spotTexArray;
	fbInit.m_depthStencilAttachment.m_loadOperation =
		AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

	U layer = 0;
	for(ShadowmapSpot& sm : m_spots)
	{
		sm.m_layerId = layer;

		fbInit.m_depthStencilAttachment.m_arrayIndex = layer;
		sm.m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

		++layer;
	}

	// Init cube layers
	m_omnis.create(getAllocator(), config.getNumber("is.sm.maxLights"));

	fbInit.m_depthStencilAttachment.m_texture = m_omniTexArray;

	layer = 0;
	for(ShadowmapOmni& sm : m_omnis)
	{
		sm.m_layerId = layer;

		for(U i = 0; i < 6; ++i)
		{
			fbInit.m_depthStencilAttachment.m_arrayIndex = layer;
			fbInit.m_depthStencilAttachment.m_faceIndex = i;
			sm.m_fb[i] = getGrManager().newInstance<Framebuffer>(fbInit);
		}

		++layer;
	}

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
Error Sm::run(SArray<SceneNode*> spotShadowCasters,
	SArray<SceneNode*> omniShadowCasters,
	CommandBufferPtr& cmdBuff)
{
	ANKI_ASSERT(m_enabled);
	ANKI_TRACE_START_EVENT(RENDER_SM);

	if(omniShadowCasters.getSize() > m_omnis.getSize()
		|| spotShadowCasters.getSize() > m_spots.getSize())
	{
		ANKI_LOGW("Too many shadow casters");
	}

	// render all
	for(SceneNode* node : spotShadowCasters)
	{
		ANKI_CHECK(doSpotLight(*node, cmdBuff));
	}

	for(SceneNode* node : omniShadowCasters)
	{
		ANKI_CHECK(doOmniLight(*node, cmdBuff));
	}

	ANKI_TRACE_STOP_EVENT(RENDER_SM);
	return ErrorCode::NONE;
}

//==============================================================================
template<typename TShadowmap, typename TContainer>
void Sm::bestCandidate(SceneNode& light, TContainer& arr, TShadowmap*& out)
{
	// Allready there
	for(TShadowmap& sm : arr)
	{
		if(&light == sm.m_light)
		{
			out = &sm;
			return;
		}
	}

	// Find a null
	for(TShadowmap& sm : arr)
	{
		if(sm.m_light == nullptr)
		{
			sm.m_light = &light;
			sm.m_timestamp = 0;
			out = &sm;
			return;
		}
	}

	// Find an old and replace it
	TShadowmap* sm = &arr[0];
	for(U i = 1; i < arr.getSize(); i++)
	{
		if(arr[i].m_timestamp < sm->m_timestamp)
		{
			sm = &arr[i];
		}
	}

	sm->m_light = &light;
	sm->m_timestamp = 0;
	out = sm;
}

//==============================================================================
Bool Sm::skip(SceneNode& light, ShadowmapBase& sm)
{
	MoveComponent* movc = light.tryGetComponent<MoveComponent>();

	Timestamp lastUpdate = movc->getTimestamp();

	Error err = light.iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& fr) {
			lastUpdate = max(lastUpdate, fr.getTimestamp());

			VisibilityTestResults& vi = fr.getVisibilityTestResults();
			lastUpdate = max(lastUpdate, vi.getShapeUpdateTimestamp());

			return ErrorCode::NONE;
		});
	(void)err;

	Bool shouldUpdate = lastUpdate >= sm.m_timestamp;
	if(shouldUpdate)
	{
		sm.m_timestamp = getGlobalTimestamp();
		LightComponent& lcomp = light.getComponent<LightComponent>();
		lcomp.setShadowMapIndex(sm.m_layerId);
	}

	return !shouldUpdate;
}

//==============================================================================
Error Sm::doSpotLight(SceneNode& light, CommandBufferPtr& cmdBuff)
{
	ShadowmapSpot* sm;
	bestCandidate(light, m_spots, sm);

	if(skip(light, *sm))
	{
		return ErrorCode::NONE;
	}

	cmdBuff->bindFramebuffer(sm->m_fb);
	cmdBuff->setViewport(0, 0, m_resolution, m_resolution);

	FrustumComponent& fr = light.getComponent<FrustumComponent>();
	SArray<CommandBufferPtr> cmdbs(&cmdBuff, 1);
	ANKI_CHECK(m_r->getSceneDrawer().render(
		fr, RenderingStage::MATERIAL, Pass::SM, cmdbs));

	ANKI_TRACE_INC_COUNTER(RENDERER_SHADOW_PASSES, 1);
	return ErrorCode::NONE;
}

//==============================================================================
Error Sm::doOmniLight(SceneNode& light, CommandBufferPtr& cmdBuff)
{
	ShadowmapOmni* sm;
	bestCandidate(light, m_omnis, sm);

	if(skip(light, *sm))
	{
		return ErrorCode::NONE;
	}

	cmdBuff->setViewport(0, 0, m_resolution, m_resolution);
	U frCount = 0;

	Error err = light.iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& fr) -> Error {
			cmdBuff->bindFramebuffer(sm->m_fb[frCount]);

			SArray<CommandBufferPtr> cmdbs(&cmdBuff, 1);
			ANKI_CHECK(m_r->getSceneDrawer().render(
				fr, RenderingStage::MATERIAL, Pass::SM, cmdbs));

			++frCount;
			return ErrorCode::NONE;
		});

	ANKI_TRACE_INC_COUNTER(RENDERER_SHADOW_PASSES, 6);

	return err;
}

} // end namespace anki
