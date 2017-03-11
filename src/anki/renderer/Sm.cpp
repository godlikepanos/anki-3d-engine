// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Sm.h>
#include <anki/renderer/Renderer.h>
#include <anki/core/App.h>
#include <anki/core/Trace.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Light.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/ThreadPool.h>

namespace anki
{

const PixelFormat Sm::DEPTH_RT_PIXEL_FORMAT(ComponentFormat::D16, TransformFormat::UNORM);

Sm::~Sm()
{
	m_spots.destroy(getAllocator());
	m_omnis.destroy(getAllocator());
}

Error Sm::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing shadowmapping");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadowmapping");
	}

	return err;
}

Error Sm::initInternal(const ConfigSet& config)
{
	m_poissonEnabled = config.getNumber("sm.poissonEnabled");
	m_bilinearEnabled = config.getNumber("sm.bilinearEnabled");
	m_resolution = config.getNumber("sm.resolution");

	//
	// Init the shadowmaps
	//

	// Create shadowmaps array
	TextureInitInfo sminit;
	sminit.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	sminit.m_usageWhenEncountered = TextureUsageBit::SAMPLED_FRAGMENT;
	sminit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	sminit.m_type = TextureType::_2D_ARRAY;
	sminit.m_width = m_resolution;
	sminit.m_height = m_resolution;
	sminit.m_layerCount = config.getNumber("sm.maxLights");
	sminit.m_depth = 1;
	sminit.m_format = DEPTH_RT_PIXEL_FORMAT;
	sminit.m_mipmapsCount = 1;
	sminit.m_sampling.m_minMagFilter = m_bilinearEnabled ? SamplingFilter::LINEAR : SamplingFilter::NEAREST;
	sminit.m_sampling.m_compareOperation = CompareOperation::LESS_EQUAL;

	m_spotTexArray = m_r->createAndClearRenderTarget(sminit);

	sminit.m_type = TextureType::CUBE_ARRAY;
	m_omniTexArray = m_r->createAndClearRenderTarget(sminit);

	// Init 2D layers
	m_spots.create(getAllocator(), config.getNumber("sm.maxLights"));

	FramebufferInitInfo fbInit;
	fbInit.m_depthStencilAttachment.m_texture = m_spotTexArray;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

	U layer = 0;
	for(ShadowmapSpot& sm : m_spots)
	{
		sm.m_layerId = layer;

		fbInit.m_depthStencilAttachment.m_surface.m_layer = layer;
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
			fbInit.m_depthStencilAttachment.m_surface.m_layer = layer;
			fbInit.m_depthStencilAttachment.m_surface.m_face = i;
			sm.m_fb[i] = getGrManager().newInstance<Framebuffer>(fbInit);
		}

		++layer;
	}

	return ErrorCode::NONE;
}

void Sm::run(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	const U threadCount = m_r->getThreadPool().getThreadsCount();

	// Spot lights
	for(U i = 0; i < ctx.m_sm.m_spots.getSize(); ++i)
	{
		cmdb->beginRenderPass(ctx.m_sm.m_spotFramebuffers[i]);
		for(U j = 0; j < threadCount; ++j)
		{
			U idx = i * threadCount + j;
			CommandBufferPtr& cmdb2 = ctx.m_sm.m_spotCommandBuffers[idx];
			if(cmdb2.isCreated())
			{
				cmdb->pushSecondLevelCommandBuffer(cmdb2);
			}
		}
		cmdb->endRenderPass();
	}

	// Omni lights
	for(U i = 0; i < ctx.m_sm.m_omnis.getSize(); ++i)
	{
		for(U j = 0; j < 6; ++j)
		{
			cmdb->beginRenderPass(ctx.m_sm.m_omniFramebuffers[i][j]);

			for(U k = 0; k < threadCount; ++k)
			{
				U idx = i * threadCount * 6 + k * 6 + j;
				CommandBufferPtr& cmdb2 = ctx.m_sm.m_omniCommandBuffers[idx];
				if(cmdb2.isCreated())
				{
					cmdb->pushSecondLevelCommandBuffer(cmdb2);
				}
			}
			cmdb->endRenderPass();
		}
	}

	ANKI_TRACE_STOP_EVENT(RENDER_SM);
}

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

Bool Sm::skip(SceneNode& light, ShadowmapBase& sm)
{
	MoveComponent* movc = light.tryGetComponent<MoveComponent>();

	Timestamp lastUpdate = movc->getTimestamp();

	Error err = light.iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) {
		lastUpdate = max(lastUpdate, fr.getTimestamp());

		VisibilityTestResults& vi = fr.getVisibilityTestResults();
		lastUpdate = max(lastUpdate, vi.getShapeUpdateTimestamp());

		return ErrorCode::NONE;
	});
	(void)err;

	Bool shouldUpdate = lastUpdate >= sm.m_timestamp || m_r->resourcesLoaded();
	if(shouldUpdate)
	{
		sm.m_timestamp = m_r->getGlobalTimestamp();
	}

	// Update the layer ID anyway
	LightComponent& lcomp = light.getComponent<LightComponent>();
	lcomp.setShadowMapIndex(sm.m_layerId);

	return !shouldUpdate;
}

Error Sm::buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const
{
	ANKI_TRACE_START_EVENT(RENDER_SM);

	for(U i = 0; i < ctx.m_sm.m_spots.getSize(); ++i)
	{
		U idx = i * threadCount + threadId;

		ANKI_CHECK(doSpotLight(*ctx.m_sm.m_spots[i],
			ctx.m_sm.m_spotCommandBuffers[idx],
			ctx.m_sm.m_spotFramebuffers[i],
			threadId,
			threadCount));
	}

	for(U i = 0; i < ctx.m_sm.m_omnis.getSize(); ++i)
	{
		U idx = i * threadCount * 6 + threadId * 6 + 0;

		ANKI_CHECK(doOmniLight(*ctx.m_sm.m_omnis[i],
			&ctx.m_sm.m_omniCommandBuffers[idx],
			ctx.m_sm.m_omniFramebuffers[i],
			threadId,
			threadCount));
	}

	ANKI_TRACE_STOP_EVENT(RENDER_SM);
	return ErrorCode::NONE;
}

Error Sm::doSpotLight(SceneNode& light, CommandBufferPtr& cmdb, FramebufferPtr& fb, U threadId, U threadCount) const
{
	FrustumComponent& frc = light.getComponent<FrustumComponent>();
	VisibilityTestResults& vis = frc.getVisibilityTestResults();
	U problemSize = vis.getCount(VisibilityGroupType::RENDERABLES_MS);
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

	if(start == end)
	{
		return ErrorCode::NONE;
	}

	CommandBufferInitInfo cinf;
	cinf.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
	cinf.m_framebuffer = fb;
	cmdb = m_r->getGrManager().newInstance<CommandBuffer>(cinf);

	// Inform on Rts
	cmdb->informTextureSurfaceCurrentUsage(m_spotTexArray,
		TextureSurfaceInfo(0, 0, 0, light.getComponent<LightComponent>().getShadowMapIndex()),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

	// Set state
	cmdb->setViewport(0, 0, m_resolution, m_resolution);
	cmdb->setPolygonOffset(1.0, 2.0);

	Error err = m_r->getSceneDrawer().drawRange(Pass::SM,
		frc.getViewMatrix(),
		frc.getViewProjectionMatrix(),
		cmdb,
		vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + start,
		vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + end);

	cmdb->flush();

	return err;
}

Error Sm::doOmniLight(
	SceneNode& light, CommandBufferPtr cmdbs[], Array<FramebufferPtr, 6>& fbs, U threadId, U threadCount) const
{
	U frCount = 0;

	Error err = light.iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
		VisibilityTestResults& vis = frc.getVisibilityTestResults();
		U problemSize = vis.getCount(VisibilityGroupType::RENDERABLES_MS);
		PtrSize start, end;
		ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

		if(start != end)
		{
			CommandBufferInitInfo cinf;
			cinf.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
			cinf.m_framebuffer = fbs[frCount];
			cmdbs[frCount] = m_r->getGrManager().newInstance<CommandBuffer>(cinf);

			// Inform on Rts
			cmdbs[frCount]->informTextureSurfaceCurrentUsage(m_omniTexArray,
				TextureSurfaceInfo(0, 0, frCount, light.getComponent<LightComponent>().getShadowMapIndex()),
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

			// Set state
			cmdbs[frCount]->setViewport(0, 0, m_resolution, m_resolution);
			cmdbs[frCount]->setPolygonOffset(1.0, 2.0);

			ANKI_CHECK(m_r->getSceneDrawer().drawRange(Pass::SM,
				frc.getViewMatrix(),
				frc.getViewProjectionMatrix(),
				cmdbs[frCount],
				vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + start,
				vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + end));

			cmdbs[frCount]->flush();
		}

		++frCount;
		return ErrorCode::NONE;
	});

	return err;
}

void Sm::prepareBuildCommandBuffers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_SM);

	// Gather the lights
	const VisibilityTestResults& vi = *ctx.m_visResults;

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
		ANKI_ASSERT(light.getLightComponentType() == LightComponentType::POINT);

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
		ANKI_ASSERT(light.getLightComponentType() == LightComponentType::SPOT);

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

	if(omniCastersCount > m_omnis.getSize() || spotCastersCount > m_spots.getSize())
	{
		ANKI_R_LOGW("Too many shadow casters");
		omniCastersCount = min<U>(omniCastersCount, m_omnis.getSize());
		spotCastersCount = min<U>(spotCastersCount, m_spots.getSize());
	}

	if(spotCastersCount > 0)
	{
		ctx.m_sm.m_spots.create(spotCastersCount);
		memcpy(&ctx.m_sm.m_spots[0], &spotCasters[0], sizeof(SceneNode*) * spotCastersCount);

		ctx.m_sm.m_spotCommandBuffers.create(spotCastersCount * m_r->getThreadPool().getThreadsCount());

		ctx.m_sm.m_spotCacheIndices.create(spotCastersCount);
#if ANKI_EXTRA_CHECKS
		memset(&ctx.m_sm.m_spotCacheIndices[0], 0xFF, sizeof(ctx.m_sm.m_spotCacheIndices[0]) * spotCastersCount);
#endif

		ctx.m_sm.m_spotFramebuffers.create(spotCastersCount);
		for(U i = 0; i < spotCastersCount; ++i)
		{
			const LightComponent& lightc = ctx.m_sm.m_spots[i]->getComponent<LightComponent>();
			const U idx = lightc.getShadowMapIndex();

			ctx.m_sm.m_spotFramebuffers[i] = m_spots[idx].m_fb;
			ctx.m_sm.m_spotCacheIndices[i] = idx;
		}
	}

	if(omniCastersCount > 0)
	{
		ctx.m_sm.m_omnis.create(omniCastersCount);
		memcpy(&ctx.m_sm.m_omnis[0], &omniCasters[0], sizeof(SceneNode*) * omniCastersCount);

		ctx.m_sm.m_omniCommandBuffers.create(omniCastersCount * 6 * m_r->getThreadPool().getThreadsCount());

		ctx.m_sm.m_omniCacheIndices.create(omniCastersCount);
#if ANKI_EXTRA_CHECKS
		memset(&ctx.m_sm.m_omniCacheIndices[0], 0xFF, sizeof(ctx.m_sm.m_omniCacheIndices[0]) * omniCastersCount);
#endif

		ctx.m_sm.m_omniFramebuffers.create(omniCastersCount);
		for(U i = 0; i < omniCastersCount; ++i)
		{
			const LightComponent& lightc = ctx.m_sm.m_omnis[i]->getComponent<LightComponent>();
			const U idx = lightc.getShadowMapIndex();

			for(U j = 0; j < 6; ++j)
			{
				ctx.m_sm.m_omniFramebuffers[i][j] = m_omnis[idx].m_fb[j];
			}

			ctx.m_sm.m_omniCacheIndices[i] = idx;
		}
	}

	ANKI_TRACE_STOP_EVENT(RENDER_SM);
}

void Sm::setPreRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Spot lights
	for(U i = 0; i < ctx.m_sm.m_spotCacheIndices.getSize(); ++i)
	{
		U layer = ctx.m_sm.m_spotCacheIndices[i];

		cmdb->setTextureSurfaceBarrier(m_spotTexArray,
			TextureUsageBit::NONE,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
			TextureSurfaceInfo(0, 0, 0, layer));
	}

	// Omni lights
	for(U i = 0; i < ctx.m_sm.m_omniCacheIndices.getSize(); ++i)
	{
		for(U j = 0; j < 6; ++j)
		{
			U layer = ctx.m_sm.m_omniCacheIndices[i];

			cmdb->setTextureSurfaceBarrier(m_omniTexArray,
				TextureUsageBit::NONE,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
				TextureSurfaceInfo(0, 0, j, layer));
		}
	}

	ANKI_TRACE_STOP_EVENT(RENDER_SM);
}

void Sm::setPostRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Spot lights
	for(U i = 0; i < ctx.m_sm.m_spotCacheIndices.getSize(); ++i)
	{
		U layer = ctx.m_sm.m_spotCacheIndices[i];

		cmdb->setTextureSurfaceBarrier(m_spotTexArray,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, layer));
	}

	// Omni lights
	for(U i = 0; i < ctx.m_sm.m_omniCacheIndices.getSize(); ++i)
	{
		for(U j = 0; j < 6; ++j)
		{
			U layer = ctx.m_sm.m_omniCacheIndices[i];

			cmdb->setTextureSurfaceBarrier(m_omniTexArray,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
				TextureUsageBit::SAMPLED_FRAGMENT,
				TextureSurfaceInfo(0, 0, j, layer));
		}
	}

	cmdb->informTextureCurrentUsage(m_spotTexArray, TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->informTextureCurrentUsage(m_omniTexArray, TextureUsageBit::SAMPLED_FRAGMENT);

	ANKI_TRACE_STOP_EVENT(RENDER_SM);
}

} // end namespace anki
