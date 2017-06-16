// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/App.h>
#include <anki/core/Trace.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/ThreadPool.h>

namespace anki
{

const PixelFormat ShadowMapping::DEPTH_RT_PIXEL_FORMAT(ComponentFormat::D16, TransformFormat::UNORM);

ShadowMapping::~ShadowMapping()
{
	m_spots.destroy(getAllocator());
	m_omnis.destroy(getAllocator());
}

Error ShadowMapping::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing shadowmapping");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadowmapping");
	}

	return err;
}

Error ShadowMapping::initInternal(const ConfigSet& config)
{
	m_poissonEnabled = config.getNumber("sm.poissonEnabled");
	m_bilinearEnabled = config.getNumber("sm.bilinearEnabled");
	m_resolution = config.getNumber("sm.resolution");

	//
	// Init the shadowmaps
	//

	// Create shadowmaps array
	TextureInitInfo sminit("shadows");
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

	FramebufferInitInfo fbInit("shadows");
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

void ShadowMapping::run(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	const U threadCount = m_r->getThreadPool().getThreadsCount();

	// Spot lights
	for(U i = 0; i < ctx.m_shadowMapping.m_spots.getSize(); ++i)
	{
		cmdb->beginRenderPass(ctx.m_shadowMapping.m_spotFramebuffers[i]);
		for(U j = 0; j < threadCount; ++j)
		{
			U idx = i * threadCount + j;
			CommandBufferPtr& cmdb2 = ctx.m_shadowMapping.m_spotCommandBuffers[idx];
			if(cmdb2.isCreated())
			{
				cmdb->pushSecondLevelCommandBuffer(cmdb2);
			}
		}
		cmdb->endRenderPass();

		ANKI_TRACE_INC_COUNTER(RENDERER_SHADOW_PASSES, 1);
	}

	// Omni lights
	for(U i = 0; i < ctx.m_shadowMapping.m_omnis.getSize(); ++i)
	{
		for(U j = 0; j < 6; ++j)
		{
			cmdb->beginRenderPass(ctx.m_shadowMapping.m_omniFramebuffers[i][j]);

			for(U k = 0; k < threadCount; ++k)
			{
				U idx = i * threadCount * 6 + k * 6 + j;
				CommandBufferPtr& cmdb2 = ctx.m_shadowMapping.m_omniCommandBuffers[idx];
				if(cmdb2.isCreated())
				{
					cmdb->pushSecondLevelCommandBuffer(cmdb2);
				}
			}
			cmdb->endRenderPass();

			ANKI_TRACE_INC_COUNTER(RENDERER_SHADOW_PASSES, 1);
		}
	}
}

template<typename TLightElement, typename TShadowmap, typename TContainer>
void ShadowMapping::bestCandidate(TLightElement& light, TContainer& arr, TShadowmap*& out)
{
	// Allready there
	for(TShadowmap& sm : arr)
	{
		if(light.m_uuid == sm.m_lightUuid)
		{
			out = &sm;
			return;
		}
	}

	// Find a null
	for(TShadowmap& sm : arr)
	{
		if(sm.m_lightUuid == 0)
		{
			sm.m_lightUuid = light.m_uuid;
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

	sm->m_lightUuid = light.m_uuid;
	sm->m_timestamp = 0;
	out = sm;
}

Bool ShadowMapping::skip(PointLightQueueElement& light, ShadowmapBase& sm)
{
	Timestamp maxTimestamp = light.m_shadowRenderQueues[0]->m_shadowRenderablesLastUpdateTimestamp;
	for(U i = 1; i < 6; ++i)
	{
		maxTimestamp = max(maxTimestamp, light.m_shadowRenderQueues[i]->m_shadowRenderablesLastUpdateTimestamp);
	}

	const Bool shouldUpdate = maxTimestamp >= sm.m_timestamp || m_r->resourcesLoaded();
	if(shouldUpdate)
	{
		sm.m_timestamp = m_r->getGlobalTimestamp();
	}

	// Update the layer ID anyway
	light.m_textureArrayIndex = sm.m_layerId;

	return !shouldUpdate;
}

Bool ShadowMapping::skip(SpotLightQueueElement& light, ShadowmapBase& sm)
{
	const Timestamp maxTimestamp = light.m_shadowRenderQueue->m_shadowRenderablesLastUpdateTimestamp;

	const Bool shouldUpdate = maxTimestamp >= sm.m_timestamp || m_r->resourcesLoaded();
	if(shouldUpdate)
	{
		sm.m_timestamp = m_r->getGlobalTimestamp();
	}

	// Update the layer ID anyway
	light.m_textureArrayIndex = sm.m_layerId;

	return !shouldUpdate;
}

void ShadowMapping::buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);

	for(U i = 0; i < ctx.m_shadowMapping.m_spots.getSize(); ++i)
	{
		U idx = i * threadCount + threadId;

		doSpotLight(*ctx.m_shadowMapping.m_spots[i],
			ctx.m_shadowMapping.m_spotCommandBuffers[idx],
			ctx.m_shadowMapping.m_spotFramebuffers[i],
			threadId,
			threadCount);
	}

	for(U i = 0; i < ctx.m_shadowMapping.m_omnis.getSize(); ++i)
	{
		U idx = i * threadCount * 6 + threadId * 6 + 0;

		doOmniLight(*ctx.m_shadowMapping.m_omnis[i],
			&ctx.m_shadowMapping.m_omniCommandBuffers[idx],
			ctx.m_shadowMapping.m_omniFramebuffers[i],
			threadId,
			threadCount);
	}
}

void ShadowMapping::doSpotLight(
	const SpotLightQueueElement& light, CommandBufferPtr& cmdb, FramebufferPtr& fb, U threadId, U threadCount) const
{
	const U problemSize = light.m_shadowRenderQueue->m_renderables.getSize();
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

	if(start == end)
	{
		return;
	}

	CommandBufferInitInfo cinf;
	cinf.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
	cinf.m_framebuffer = fb;
	if(end - start < COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS)
	{
		cinf.m_flags |= CommandBufferFlag::SMALL_BATCH;
	}
	cmdb = m_r->getGrManager().newInstance<CommandBuffer>(cinf);

	// Inform on Rts
	cmdb->informTextureSurfaceCurrentUsage(m_spotTexArray,
		TextureSurfaceInfo(0, 0, 0, light.m_textureArrayIndex),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

	// Set state
	cmdb->setViewport(0, 0, m_resolution, m_resolution);
	cmdb->setPolygonOffset(1.0, 2.0);

	m_r->getSceneDrawer().drawRange(Pass::SM,
		light.m_shadowRenderQueue->m_viewMatrix,
		light.m_shadowRenderQueue->m_viewProjectionMatrix,
		cmdb,
		light.m_shadowRenderQueue->m_renderables.getBegin() + start,
		light.m_shadowRenderQueue->m_renderables.getBegin() + end);

	cmdb->flush();
}

void ShadowMapping::doOmniLight(const PointLightQueueElement& light,
	CommandBufferPtr cmdbs[],
	Array<FramebufferPtr, 6>& fbs,
	U threadId,
	U threadCount) const
{
	for(U i = 0; i < 6; ++i)
	{
		const RenderQueue& rqueue = *light.m_shadowRenderQueues[i];

		const U problemSize = rqueue.m_renderables.getSize();
		PtrSize start, end;
		ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

		if(start != end)
		{
			CommandBufferInitInfo cinf;
			cinf.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
			if(end - start < COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS)
			{
				cinf.m_flags |= CommandBufferFlag::SMALL_BATCH;
			}
			cinf.m_framebuffer = fbs[i];
			cmdbs[i] = m_r->getGrManager().newInstance<CommandBuffer>(cinf);

			// Inform on Rts
			cmdbs[i]->informTextureSurfaceCurrentUsage(m_omniTexArray,
				TextureSurfaceInfo(0, 0, i, light.m_textureArrayIndex),
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

			// Set state
			cmdbs[i]->setViewport(0, 0, m_resolution, m_resolution);
			cmdbs[i]->setPolygonOffset(1.0, 2.0);

			m_r->getSceneDrawer().drawRange(Pass::SM,
				rqueue.m_viewMatrix,
				rqueue.m_viewProjectionMatrix,
				cmdbs[i],
				rqueue.m_renderables.getBegin() + start,
				rqueue.m_renderables.getBegin() + end);

			cmdbs[i]->flush();
		}
	}
}

void ShadowMapping::prepareBuildCommandBuffers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);

	// Gather the lights
	RenderQueue& rqueue = *ctx.m_renderQueue;

	const U MAX = 64;
	Array<SpotLightQueueElement*, MAX> spotCasters;
	Array<PointLightQueueElement*, MAX> omniCasters;
	U spotCastersCount = 0;
	U omniCastersCount = 0;

	{
		auto it = rqueue.m_pointLights.getBegin();
		auto lend = rqueue.m_pointLights.getEnd();
		for(; it != lend; ++it)
		{
			const Bool castsShadow = it->m_shadowRenderQueues[0] != nullptr;

			if(castsShadow)
			{
				ShadowmapOmni* sm;
				bestCandidate(*it, m_omnis, sm);

				if(!skip(*it, *sm))
				{
					omniCasters[omniCastersCount++] = &(*it);
				}
			}
		}
	}

	{
		auto it = rqueue.m_spotLights.getBegin();
		auto lend = rqueue.m_spotLights.getEnd();
		for(; it != lend; ++it)
		{
			const Bool castsShadow = it->m_shadowRenderQueue != nullptr;

			if(castsShadow)
			{
				ShadowmapSpot* sm;
				bestCandidate(*it, m_spots, sm);

				if(!skip(*it, *sm))
				{
					spotCasters[spotCastersCount++] = &(*it);
				}
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
		ctx.m_shadowMapping.m_spots.create(spotCastersCount);
		memcpy(&ctx.m_shadowMapping.m_spots[0], &spotCasters[0], sizeof(SpotLightQueueElement*) * spotCastersCount);

		ctx.m_shadowMapping.m_spotCommandBuffers.create(spotCastersCount * m_r->getThreadPool().getThreadsCount());

		ctx.m_shadowMapping.m_spotCacheIndices.create(spotCastersCount);
#if ANKI_EXTRA_CHECKS
		memset(&ctx.m_shadowMapping.m_spotCacheIndices[0],
			0xFF,
			sizeof(ctx.m_shadowMapping.m_spotCacheIndices[0]) * spotCastersCount);
#endif

		ctx.m_shadowMapping.m_spotFramebuffers.create(spotCastersCount);
		for(U i = 0; i < spotCastersCount; ++i)
		{
			const U idx = ctx.m_shadowMapping.m_spots[i]->m_textureArrayIndex;

			ctx.m_shadowMapping.m_spotFramebuffers[i] = m_spots[idx].m_fb;
			ctx.m_shadowMapping.m_spotCacheIndices[i] = idx;
		}
	}

	if(omniCastersCount > 0)
	{
		ctx.m_shadowMapping.m_omnis.create(omniCastersCount);
		memcpy(&ctx.m_shadowMapping.m_omnis[0], &omniCasters[0], sizeof(PointLightQueueElement*) * omniCastersCount);

		ctx.m_shadowMapping.m_omniCommandBuffers.create(omniCastersCount * 6 * m_r->getThreadPool().getThreadsCount());

		ctx.m_shadowMapping.m_omniCacheIndices.create(omniCastersCount);
#if ANKI_EXTRA_CHECKS
		memset(&ctx.m_shadowMapping.m_omniCacheIndices[0],
			0xFF,
			sizeof(ctx.m_shadowMapping.m_omniCacheIndices[0]) * omniCastersCount);
#endif

		ctx.m_shadowMapping.m_omniFramebuffers.create(omniCastersCount);
		for(U i = 0; i < omniCastersCount; ++i)
		{
			const U idx = ctx.m_shadowMapping.m_omnis[i]->m_textureArrayIndex;

			for(U j = 0; j < 6; ++j)
			{
				ctx.m_shadowMapping.m_omniFramebuffers[i][j] = m_omnis[idx].m_fb[j];
			}

			ctx.m_shadowMapping.m_omniCacheIndices[i] = idx;
		}
	}
}

void ShadowMapping::setPreRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Spot lights
	for(U i = 0; i < ctx.m_shadowMapping.m_spotCacheIndices.getSize(); ++i)
	{
		U layer = ctx.m_shadowMapping.m_spotCacheIndices[i];

		cmdb->setTextureSurfaceBarrier(m_spotTexArray,
			TextureUsageBit::NONE,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
			TextureSurfaceInfo(0, 0, 0, layer));
	}

	// Omni lights
	for(U i = 0; i < ctx.m_shadowMapping.m_omniCacheIndices.getSize(); ++i)
	{
		for(U j = 0; j < 6; ++j)
		{
			U layer = ctx.m_shadowMapping.m_omniCacheIndices[i];

			cmdb->setTextureSurfaceBarrier(m_omniTexArray,
				TextureUsageBit::NONE,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
				TextureSurfaceInfo(0, 0, j, layer));
		}
	}
}

void ShadowMapping::setPostRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Spot lights
	for(U i = 0; i < ctx.m_shadowMapping.m_spotCacheIndices.getSize(); ++i)
	{
		U layer = ctx.m_shadowMapping.m_spotCacheIndices[i];

		cmdb->setTextureSurfaceBarrier(m_spotTexArray,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, layer));
	}

	// Omni lights
	for(U i = 0; i < ctx.m_shadowMapping.m_omniCacheIndices.getSize(); ++i)
	{
		for(U j = 0; j < 6; ++j)
		{
			U layer = ctx.m_shadowMapping.m_omniCacheIndices[i];

			cmdb->setTextureSurfaceBarrier(m_omniTexArray,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
				TextureUsageBit::SAMPLED_FRAGMENT,
				TextureSurfaceInfo(0, 0, j, layer));
		}
	}

	cmdb->informTextureCurrentUsage(m_spotTexArray, TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->informTextureCurrentUsage(m_omniTexArray, TextureUsageBit::SAMPLED_FRAGMENT);
}

} // end namespace anki
