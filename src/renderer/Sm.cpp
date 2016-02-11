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
	m_poissonEnabled = config.getNumber("sm.poissonEnabled");
	m_bilinearEnabled = config.getNumber("sm.bilinearEnabled");
	m_resolution = config.getNumber("sm.resolution");

	//
	// Init the shadowmaps
	//

	// Create shadowmaps array
	TextureInitInfo sminit;
	sminit.m_type = TextureType::_2D_ARRAY;
	sminit.m_width = m_resolution;
	sminit.m_height = m_resolution;
	sminit.m_depth = config.getNumber("sm.maxLights");
	sminit.m_format = DEPTH_RT_PIXEL_FORMAT;
	sminit.m_mipmapsCount = 1;
	sminit.m_sampling.m_minMagFilter =
		m_bilinearEnabled ? SamplingFilter::LINEAR : SamplingFilter::NEAREST;
	sminit.m_sampling.m_compareOperation = CompareOperation::LESS_EQUAL;

	m_spotTexArray = getGrManager().newInstance<Texture>(sminit);

	sminit.m_type = TextureType::CUBE_ARRAY;
	m_omniTexArray = getGrManager().newInstance<Texture>(sminit);

	// Init 2D layers
	m_spots.create(getAllocator(), config.getNumber("sm.maxLights"));

	FramebufferInitInfo fbInit;
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
	m_omnis.create(getAllocator(), config.getNumber("sm.maxLights"));

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
void Sm::run(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	const U threadCount = m_r->getThreadPool().getThreadsCount();

	// Spot lights
	for(U i = 0; i < ctx.m_sm.m_spots.getSize(); ++i)
	{
		cmdb->bindFramebuffer(ctx.m_sm.m_spotFramebuffers[i]);
		for(U j = 0; j < threadCount; ++j)
		{
			U idx = i * threadCount + j;
			CommandBufferPtr& cmdb2 = ctx.m_sm.m_spotCommandBuffers[idx];
			if(cmdb2.isCreated())
			{
				cmdb->pushSecondLevelCommandBuffer(cmdb2);
			}
		}
	}

	// Omni lights
	for(U i = 0; i < ctx.m_sm.m_omnis.getSize(); ++i)
	{
		for(U j = 0; j < 6; ++j)
		{
			cmdb->bindFramebuffer(ctx.m_sm.m_omniFramebuffers[i][j]);

			for(U k = 0; k < threadCount; ++k)
			{
				U idx = i * threadCount * 6 + k * 6 + j;
				CommandBufferPtr& cmdb2 = ctx.m_sm.m_omniCommandBuffers[idx];
				if(cmdb2.isCreated())
				{
					cmdb->pushSecondLevelCommandBuffer(cmdb2);
				}
			}
		}
	}

	ANKI_TRACE_STOP_EVENT(RENDER_SM);
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
		sm.m_timestamp = m_r->getGlobalTimestamp();
	}

	// Update the layer ID anyway
	LightComponent& lcomp = light.getComponent<LightComponent>();
	lcomp.setShadowMapIndex(sm.m_layerId);

	return !shouldUpdate;
}

//==============================================================================
Error Sm::buildCommandBuffers(
	RenderingContext& ctx, U threadId, U threadCount) const
{
	ANKI_TRACE_START_EVENT(RENDER_SM);

	for(U i = 0; i < ctx.m_sm.m_spots.getSize(); ++i)
	{
		U idx = i * threadCount + threadId;

		ANKI_CHECK(doSpotLight(*ctx.m_sm.m_spots[i],
			ctx.m_sm.m_spotCommandBuffers[idx],
			threadId,
			threadCount));
	}

	for(U i = 0; i < ctx.m_sm.m_omnis.getSize(); ++i)
	{
		U idx = i * threadCount * 6 + threadId * 6 + 0;

		ANKI_CHECK(doOmniLight(*ctx.m_sm.m_omnis[i],
			&ctx.m_sm.m_omniCommandBuffers[idx],
			threadId,
			threadCount));
	}

	ANKI_TRACE_STOP_EVENT(RENDER_SM);
	return ErrorCode::NONE;
}

//==============================================================================
Error Sm::doSpotLight(
	SceneNode& light, CommandBufferPtr& cmdb, U threadId, U threadCount) const
{
	FrustumComponent& frc = light.getComponent<FrustumComponent>();
	VisibilityTestResults& vis = frc.getVisibilityTestResults();
	U problemSize = vis.getCount(VisibilityGroupType::RENDERABLES_MS);
	PtrSize start, end;
	ThreadPool::Task::choseStartEnd(
		threadId, threadCount, problemSize, start, end);

	if(start == end)
	{
		return ErrorCode::NONE;
	}

	cmdb = m_r->getGrManager().newInstance<CommandBuffer>();
	cmdb->setViewport(0, 0, m_resolution, m_resolution);
	cmdb->setPolygonOffset(1.0, 2.0);

	Error err = m_r->getSceneDrawer().drawRange(Pass::SM,
		frc,
		cmdb,
		vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + start,
		vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + end);

	return err;
}

//==============================================================================
Error Sm::doOmniLight(
	SceneNode& light, CommandBufferPtr cmdbs[], U threadId, U threadCount) const
{
	U frCount = 0;

	Error err = light.iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& frc) -> Error {
			VisibilityTestResults& vis = frc.getVisibilityTestResults();
			U problemSize = vis.getCount(VisibilityGroupType::RENDERABLES_MS);
			PtrSize start, end;
			ThreadPool::Task::choseStartEnd(
				threadId, threadCount, problemSize, start, end);

			if(start != end)
			{
				cmdbs[frCount] = getGrManager().newInstance<CommandBuffer>();
				cmdbs[frCount]->setViewport(0, 0, m_resolution, m_resolution);
				cmdbs[frCount]->setPolygonOffset(1.0, 2.0);

				ANKI_CHECK(m_r->getSceneDrawer().drawRange(Pass::SM,
					frc,
					cmdbs[frCount],
					vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + start,
					vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + end));
			}

			++frCount;
			return ErrorCode::NONE;
		});

	return err;
}

//==============================================================================
void Sm::prepareBuildCommandBuffers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_SM);

	// Gather the lights
	VisibilityTestResults& vi =
		ctx.m_frustumComponent->getVisibilityTestResults();

	const U MAX = 64;
	Array<SceneNode*, MAX> spotCasters;
	Array<SceneNode*, MAX> omniCasters;
	U spotCastersCount = 0;
	U omniCastersCount = 0;

	auto it = vi.getBegin(VisibilityGroupType::LIGHTS_POINT);
	auto lend = vi.getEnd(VisibilityGroupType::LIGHTS_POINT);
	for(; it != lend; ++it)
	{
		SceneNode* node = (*it).m_node;
		LightComponent& light = node->getComponent<LightComponent>();
		ANKI_ASSERT(light.getLightType() == LightComponent::LightType::POINT);
		
		if(light.getShadowEnabled())
		{
			ShadowmapOmni* sm;
			bestCandidate(*node, m_omnis, sm);

			if(!skip(*node, *sm))
			{
				omniCasters[omniCastersCount++] = node;
			}
		}
	}

	it = vi.getBegin(VisibilityGroupType::LIGHTS_SPOT);
	lend = vi.getEnd(VisibilityGroupType::LIGHTS_SPOT);
	for(; it != lend; ++it)
	{
		SceneNode* node = (*it).m_node;
		LightComponent& light = node->getComponent<LightComponent>();
		ANKI_ASSERT(light.getLightType() == LightComponent::LightType::SPOT);

		if(light.getShadowEnabled())
		{
			ShadowmapSpot* sm;
			bestCandidate(*node, m_spots, sm);

			if(!skip(*node, *sm))
			{
				spotCasters[spotCastersCount++] = node;
			}
		}
	}

	if(omniCastersCount > m_omnis.getSize()
		|| spotCastersCount > m_spots.getSize())
	{
		ANKI_LOGW("Too many shadow casters");
		omniCastersCount = min(omniCastersCount, m_omnis.getSize());
		spotCastersCount = min(spotCastersCount, m_spots.getSize());
	}

	if(spotCastersCount > 0)
	{
		ctx.m_sm.m_spots.create(spotCastersCount);
		memcpy(&ctx.m_sm.m_spots[0],
			&spotCasters[0],
			sizeof(SceneNode*) * spotCastersCount);

		ctx.m_sm.m_spotCommandBuffers.create(
			spotCastersCount * m_r->getThreadPool().getThreadsCount());

		ctx.m_sm.m_spotFramebuffers.create(spotCastersCount);
		for(U i = 0; i < spotCastersCount; ++i)
		{
			const LightComponent& lightc =
				ctx.m_sm.m_spots[i]->getComponent<LightComponent>();
			U idx = lightc.getShadowMapIndex();

			ctx.m_sm.m_spotFramebuffers[i] = m_spots[idx].m_fb;
		}
	}

	if(omniCastersCount > 0)
	{
		ctx.m_sm.m_omnis.create(omniCastersCount);
		memcpy(&ctx.m_sm.m_omnis[0],
			&omniCasters[0],
			sizeof(SceneNode*) * omniCastersCount);

		ctx.m_sm.m_omniCommandBuffers.create(
			omniCastersCount * 6 * m_r->getThreadPool().getThreadsCount());

		ctx.m_sm.m_omniFramebuffers.create(omniCastersCount);
		for(U i = 0; i < omniCastersCount; ++i)
		{
			const LightComponent& lightc =
				ctx.m_sm.m_omnis[i]->getComponent<LightComponent>();
			U idx = lightc.getShadowMapIndex();

			for(U j = 0; j < 6; ++j)
			{
				ctx.m_sm.m_omniFramebuffers[i][j] = m_omnis[idx].m_fb[j];
			}
		}
	}

	ANKI_TRACE_STOP_EVENT(RENDER_SM);
}

} // end namespace anki
