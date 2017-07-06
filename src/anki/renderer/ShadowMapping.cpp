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
	m_tileResolution = config.getNumber("r.shadowMapping.resolution");

	// Align the count
	U spotLightMaxCount = ceil(sqrt(config.getNumber("r.shadowMapping.maxLights")));
	m_atlasResolution = spotLightMaxCount * m_tileResolution;
	spotLightMaxCount *= spotLightMaxCount;

	m_scratchSm.m_batchSize = config.getNumber("r.shadowMapping.batchCount");
	ANKI_ASSERT(m_scratchSm.m_batchSize > 0);

	//
	// Init the shadowmaps and FBs
	//
	{
		m_scratchSm.m_rt = m_r->createAndClearRenderTarget(
			m_r->create2DRenderTargetInitInfo(m_tileResolution * m_scratchSm.m_batchSize,
				m_tileResolution,
				SHADOW_DEPTH_PIXEL_FORMAT,
				TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
				SamplingFilter::LINEAR,
				1,
				"shadows"));

		FramebufferInitInfo fbInit("shadows");
		fbInit.m_depthStencilAttachment.m_texture = m_scratchSm.m_rt;
		fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
		fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;
		m_scratchSm.m_fb = getGrManager().newInstance<Framebuffer>(fbInit);
	}

	//
	// Init the vertical ESM textures and FBs
	//

	// Spot
	{
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(m_atlasResolution,
			m_atlasResolution,
			SHADOW_COLOR_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			SamplingFilter::LINEAR,
			1,
			"esmspot");
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		m_spotTex = m_r->createAndClearRenderTarget(texinit);

		FramebufferInitInfo fbInit("esmspot");
		fbInit.m_colorAttachments[0].m_texture = m_spotTex;
		fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		fbInit.m_colorAttachmentCount = 1;
		m_spotsFb = getGrManager().newInstance<Framebuffer>(fbInit);

		// Init cache entries
		m_spots.create(getAllocator(), spotLightMaxCount);
		U count = 0;
		for(SpotCacheEntry& sm : m_spots)
		{
			const U i = count % (m_atlasResolution / m_tileResolution);
			const U j = count / (m_atlasResolution / m_tileResolution);

			sm.m_renderArea[0] = i * m_tileResolution;
			sm.m_renderArea[1] = sm.m_renderArea[0] + m_tileResolution;
			sm.m_renderArea[2] = j * m_tileResolution;
			sm.m_renderArea[3] = sm.m_renderArea[2] + m_tileResolution;

			sm.m_uv[0] = F32(sm.m_renderArea[0]) / F32(m_atlasResolution);
			sm.m_uv[1] = F32(sm.m_renderArea[2]) / F32(m_atlasResolution);
			sm.m_uv[2] = sm.m_uv[3] = F32(m_tileResolution) / F32(m_atlasResolution); // Width & height

			++count;
		}
	}

	// Omni
	{
		TextureInitInfo sminit = m_r->create2DRenderTargetInitInfo(m_tileResolution,
			m_tileResolution,
			SHADOW_COLOR_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			SamplingFilter::LINEAR,
			1,
			"esmvpoint");

		sminit.m_layerCount = config.getNumber("r.shadowMapping.maxLights");
		sminit.m_type = TextureType::CUBE_ARRAY;
		m_omniTexArray = m_r->createAndClearRenderTarget(sminit);

		// Init cache entries
		m_omnis.create(getAllocator(), config.getNumber("r.shadowMapping.maxLights"));

		FramebufferInitInfo fbInit("esmpoint");
		fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		fbInit.m_colorAttachments[0].m_texture = m_omniTexArray;
		fbInit.m_colorAttachmentCount = 1;

		U count = 0;
		for(OmniCacheEntry& sm : m_omnis)
		{
			sm.m_layerId = count;

			for(U f = 0; f < 6; ++f)
			{
				fbInit.m_colorAttachments[0].m_surface.m_layer = count;
				fbInit.m_colorAttachments[0].m_surface.m_face = f;
				sm.m_fbs[f] = getGrManager().newInstance<Framebuffer>(fbInit);
			}

			++count;
		}
	}

	//
	// Init prog
	//
	ANKI_CHECK(
		getResourceManager().loadResource("programs/ExponentialShadowmappingResolve.ankiprog", m_esmResolveProg));

	ShaderProgramResourceConstantValueInitList<1> consts(m_esmResolveProg);
	consts.add("INPUT_TEXTURE_SIZE", UVec2(m_tileResolution * m_scratchSm.m_batchSize, m_tileResolution));

	const ShaderProgramResourceVariant* variant;
	m_esmResolveProg->getOrCreateVariant(consts.get(), variant);
	m_esmResolveGrProg = variant->getProgram();

	return ErrorCode::NONE;
}

void ShadowMapping::run(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	const U threadCount = m_r->getThreadPool().getThreadsCount();
	const U spotPassCount = ctx.m_shadowMapping.m_spotCasters.getSize();
	const U omniPassCount = ctx.m_shadowMapping.m_omniCasters.getSize() * 6;
	const U totalShadowPassCount = spotPassCount + omniPassCount;

	// Iterate the batches
	for(U b = 0; b < totalShadowPassCount; b += m_scratchSm.m_batchSize)
	{
		const U begin = b;
		const U end = min(b + m_scratchSm.m_batchSize, totalShadowPassCount);

		const U crntBatchSize = end - begin;
		ANKI_ASSERT(crntBatchSize <= m_scratchSm.m_batchSize);

		// The limits of every light type
		const U spotBegin = b;
		const U spotEnd = min(b + m_scratchSm.m_batchSize, spotPassCount);

		const U omniBegin = max(b, spotPassCount);
		const U omniEnd = min(b + m_scratchSm.m_batchSize, totalShadowPassCount);

		//
		// Barriers
		//
		if(b > 0)
		{
			cmdb->setTextureSurfaceBarrier(m_scratchSm.m_rt,
				TextureUsageBit::SAMPLED_FRAGMENT,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
				TextureSurfaceInfo(0, 0, 0, 0));
		}

		//
		// Do shadow mapping for the batch
		//
		cmdb->beginRenderPass(m_scratchSm.m_fb, 0, 0, crntBatchSize * m_tileResolution, m_tileResolution);

		for(U i = spotBegin; i < spotEnd; ++i)
		{
			RenderingContext::ShadowMapping::SpotCasterInfo& inf = ctx.m_shadowMapping.m_spotCasters[i];
			for(CommandBufferPtr& cmdb2 : inf.m_cmdbs)
			{
				if(cmdb2.isCreated())
				{
					cmdb->pushSecondLevelCommandBuffer(cmdb2);
				}
			}
		}

		for(U i = omniBegin; i < omniEnd; ++i)
		{
			const U infIdx = (i - spotPassCount) / 6;
			const U faceIdx = (i - spotPassCount) % 6;

			RenderingContext::ShadowMapping::OmniCasterInfo& inf = ctx.m_shadowMapping.m_omniCasters[infIdx];
			for(U t = 0; t < threadCount; ++t)
			{
				CommandBufferPtr& cmdb2 = inf.m_cmdbs[6 * t + faceIdx];
				if(cmdb2.isCreated())
				{
					cmdb->pushSecondLevelCommandBuffer(cmdb2);
				}
			}
		}

		cmdb->endRenderPass();

		//
		// Barriers
		//
		cmdb->setTextureSurfaceBarrier(m_scratchSm.m_rt,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));

		if(b == 0 && spotBegin != spotEnd)
		{
			cmdb->setTextureSurfaceBarrier(m_spotTex,
				TextureUsageBit::SAMPLED_FRAGMENT,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
				TextureSurfaceInfo(0, 0, 0, 0));
		}

		for(U i = omniBegin; i < omniEnd; ++i)
		{
			const U omniIdx = (i - spotPassCount) / 6;
			const U faceIdx = (i - spotPassCount) % 6;

			const RenderingContext::ShadowMapping::OmniCasterInfo& inf = ctx.m_shadowMapping.m_omniCasters[omniIdx];

			cmdb->setTextureSurfaceBarrier(m_omniTexArray,
				TextureUsageBit::NONE,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
				TextureSurfaceInfo(0, 0, faceIdx, inf.m_light->m_textureArrayIndex));
		}

		//
		// Flush the result to spot atlas
		//
		cmdb->bindTexture(0, 0, m_scratchSm.m_rt);
		cmdb->bindShaderProgram(m_esmResolveGrProg);

		for(U i = spotBegin; i < spotEnd; ++i)
		{
			RenderingContext::ShadowMapping::SpotCasterInfo& inf = ctx.m_shadowMapping.m_spotCasters[i];

			cmdb->beginRenderPass(
				m_spotsFb, inf.m_renderArea[0], inf.m_renderArea[2], inf.m_renderArea[1], inf.m_renderArea[3]);
			cmdb->setViewport(inf.m_renderArea[0], inf.m_renderArea[2], inf.m_renderArea[1], inf.m_renderArea[3]);
			cmdb->setScissor(inf.m_renderArea[0], inf.m_renderArea[2], inf.m_renderArea[1], inf.m_renderArea[3]);

			Vec4* unis = allocateAndBindUniforms<Vec4*>(sizeof(Vec4) * 2, cmdb, 0, 0);
			unis[0] = Vec4(inf.m_light->m_shadowRenderQueue->m_cameraNear,
				inf.m_light->m_shadowRenderQueue->m_cameraFar,
				0.0f,
				0.0f);
			unis[1] = Vec4(1.0f / m_scratchSm.m_batchSize, 1.0f, F32(i - b) / m_scratchSm.m_batchSize, 0.0f);

			m_r->drawQuad(cmdb);
			cmdb->endRenderPass();
		}

		//
		// Flush the result to cube array
		//
		for(U i = omniBegin; i < omniEnd; ++i)
		{
			const U infIdx = (i - spotPassCount) / 6;
			const U faceIdx = (i - spotPassCount) % 6;

			RenderingContext::ShadowMapping::OmniCasterInfo& inf = ctx.m_shadowMapping.m_omniCasters[infIdx];

			cmdb->setViewport(0, 0, m_tileResolution, m_tileResolution);
			cmdb->setScissor(0, 0, m_tileResolution, m_tileResolution);
			cmdb->beginRenderPass(m_omnis[inf.m_light->m_textureArrayIndex].m_fbs[faceIdx]);

			Vec4* unis = allocateAndBindUniforms<Vec4*>(sizeof(Vec4) * 2, cmdb, 0, 0);
			unis[0] = Vec4(inf.m_light->m_shadowRenderQueues[faceIdx]->m_cameraNear,
				inf.m_light->m_shadowRenderQueues[faceIdx]->m_cameraFar,
				0.0f,
				0.0f);
			unis[1] = Vec4(1.0f / m_scratchSm.m_batchSize, 1.0f, F32(i - b) / m_scratchSm.m_batchSize, 0.0f);

			m_r->drawQuad(cmdb);
			cmdb->endRenderPass();
		}
	} // end iterate batches

	// Disable scissor
	cmdb->setScissor(0, 0, MAX_U16, MAX_U16);
}

template<typename TLightElement, typename TShadowmap, typename TContainer>
void ShadowMapping::bestCandidate(const TLightElement& light, TContainer& arr, TShadowmap*& out)
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

Bool ShadowMapping::skip(PointLightQueueElement& light, OmniCacheEntry& sm)
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

Bool ShadowMapping::skip(SpotLightQueueElement& light, SpotCacheEntry& sm)
{
	const Timestamp maxTimestamp = light.m_shadowRenderQueue->m_shadowRenderablesLastUpdateTimestamp;

	const Bool shouldUpdate = maxTimestamp >= sm.m_timestamp || m_r->resourcesLoaded();
	if(shouldUpdate)
	{
		sm.m_timestamp = m_r->getGlobalTimestamp();
	}

	// Update the texture matrix to point to the correct region in the atlas
	light.m_textureMatrix =
		Mat4(sm.m_uv[2], 0.0, 0.0, sm.m_uv[0], 0.0, sm.m_uv[3], 0.0, sm.m_uv[1], 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0)
		* light.m_textureMatrix;

	return !shouldUpdate;
}

void ShadowMapping::buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);

	for(RenderingContext::ShadowMapping::SpotCasterInfo& inf : ctx.m_shadowMapping.m_spotCasters)
	{
		doSpotLight(*inf.m_light, inf.m_cmdbs[threadId], threadId, threadCount, inf.m_batchIdx);
	}

	for(RenderingContext::ShadowMapping::OmniCasterInfo& inf : ctx.m_shadowMapping.m_omniCasters)
	{
		doOmniLight(*inf.m_light, &inf.m_cmdbs[threadId * 6], threadId, threadCount, inf.m_firstBatchIdx);
	}
}

void ShadowMapping::doSpotLight(
	const SpotLightQueueElement& light, CommandBufferPtr& cmdb, U threadId, U threadCount, U tileIdx) const
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
	cinf.m_framebuffer = m_scratchSm.m_fb;
	if(end - start < COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS)
	{
		cinf.m_flags |= CommandBufferFlag::SMALL_BATCH;
	}
	cmdb = m_r->getGrManager().newInstance<CommandBuffer>(cinf);

	// Inform on Rts
	cmdb->informTextureSurfaceCurrentUsage(
		m_scratchSm.m_rt, TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

	// Set state
	cmdb->setViewport(tileIdx * m_tileResolution, 0, (tileIdx + 1) * m_tileResolution, m_tileResolution);
	cmdb->setScissor(tileIdx * m_tileResolution, 0, (tileIdx + 1) * m_tileResolution, m_tileResolution);

	m_r->getSceneDrawer().drawRange(Pass::SM,
		light.m_shadowRenderQueue->m_viewMatrix,
		light.m_shadowRenderQueue->m_viewProjectionMatrix,
		cmdb,
		light.m_shadowRenderQueue->m_renderables.getBegin() + start,
		light.m_shadowRenderQueue->m_renderables.getBegin() + end);

	cmdb->flush();
}

void ShadowMapping::doOmniLight(
	const PointLightQueueElement& light, CommandBufferPtr cmdbs[], U threadId, U threadCount, U firstTileIdx) const
{
	for(U f = 0; f < 6; ++f)
	{
		const RenderQueue& rqueue = *light.m_shadowRenderQueues[f];

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
			cinf.m_framebuffer = m_scratchSm.m_fb;
			cmdbs[f] = m_r->getGrManager().newInstance<CommandBuffer>(cinf);

			// Inform on Rts
			cmdbs[f]->informTextureSurfaceCurrentUsage(
				m_scratchSm.m_rt, TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

			// Set state
			const U faceTileIdx = (firstTileIdx + f) % m_scratchSm.m_batchSize;
			cmdbs[f]->setViewport(
				faceTileIdx * m_tileResolution, 0, (faceTileIdx + 1) * m_tileResolution, m_tileResolution);
			cmdbs[f]->setScissor(
				faceTileIdx * m_tileResolution, 0, (faceTileIdx + 1) * m_tileResolution, m_tileResolution);

			m_r->getSceneDrawer().drawRange(Pass::SM,
				rqueue.m_viewMatrix,
				rqueue.m_viewProjectionMatrix,
				cmdbs[f],
				rqueue.m_renderables.getBegin() + start,
				rqueue.m_renderables.getBegin() + end);

			cmdbs[f]->flush();
		}
	}
}

void ShadowMapping::prepareBuildCommandBuffers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);

	// Do some checks
	//
	WeakArray<PointLightQueueElement*> pointLights;
	if(ctx.m_renderQueue->m_shadowPointLights.getSize() > m_omnis.getSize())
	{
		ANKI_R_LOGW("Too many point shadow casters");

		pointLights =
			WeakArray<PointLightQueueElement*>(ctx.m_renderQueue->m_shadowPointLights.getBegin(), m_omnis.getSize());
	}
	else
	{
		pointLights = ctx.m_renderQueue->m_shadowPointLights;
	}

	WeakArray<SpotLightQueueElement*> spotLights;
	if(ctx.m_renderQueue->m_shadowSpotLights.getSize() > m_spots.getSize())
	{
		ANKI_R_LOGW("Too many spot shadow casters");

		spotLights =
			WeakArray<SpotLightQueueElement*>(ctx.m_renderQueue->m_shadowSpotLights.getBegin(), m_spots.getSize());
	}
	else
	{
		spotLights = ctx.m_renderQueue->m_shadowSpotLights;
	}

	// Set the texture arr indices and skip some stale lights
	//
	U batchIdx = 0;
	if(spotLights.getSize())
	{
		auto alloc = getFrameAllocator();

		WeakArray<SpotLightQueueElement*> lightArr(
			alloc.newArray<SpotLightQueueElement*>(spotLights.getSize()), spotLights.getSize());
		WeakArray<U> cacheEntries(alloc.newArray<U>(spotLights.getSize()), spotLights.getSize());
		U newLightCount = 0;
		for(U i = 0; i < spotLights.getSize(); ++i)
		{
			SpotCacheEntry* sm;
			bestCandidate(*spotLights[i], m_spots, sm);

			if(!skip(*spotLights[i], *sm))
			{
				cacheEntries[newLightCount] = sm - &m_spots[0];
				lightArr[newLightCount++] = spotLights[i];
			}
		}

		if(newLightCount)
		{
			ctx.m_shadowMapping.m_spotCasters = WeakArray<RenderingContext::ShadowMapping::SpotCasterInfo>(
				alloc.newArray<RenderingContext::ShadowMapping::SpotCasterInfo>(newLightCount), newLightCount);

			for(U i = 0; i < newLightCount; ++i)
			{
				auto& x = ctx.m_shadowMapping.m_spotCasters[i];
				x.m_light = lightArr[i];
				x.m_renderArea = m_spots[cacheEntries[i]].m_renderArea;
				x.m_cmdbs = WeakArray<CommandBufferPtr>(
					alloc.newArray<CommandBufferPtr>(m_r->getThreadPool().getThreadsCount()),
					m_r->getThreadPool().getThreadsCount());
				x.m_batchIdx = (batchIdx++) % m_scratchSm.m_batchSize;
			}
		}
	}

	if(pointLights.getSize())
	{
		auto alloc = getFrameAllocator();

		WeakArray<PointLightQueueElement*> lightArr(
			alloc.newArray<PointLightQueueElement*>(pointLights.getSize()), pointLights.getSize());
		U newLightCount = 0;
		for(U i = 0; i < pointLights.getSize(); ++i)
		{
			OmniCacheEntry* sm;
			bestCandidate(*pointLights[i], m_omnis, sm);

			if(!skip(*pointLights[i], *sm))
			{
				lightArr[newLightCount++] = pointLights[i];
			}
		}

		if(newLightCount)
		{
			ctx.m_shadowMapping.m_omniCasters = WeakArray<RenderingContext::ShadowMapping::OmniCasterInfo>(
				alloc.newArray<RenderingContext::ShadowMapping::OmniCasterInfo>(newLightCount), newLightCount);

			for(U i = 0; i < newLightCount; ++i)
			{
				auto& x = ctx.m_shadowMapping.m_omniCasters[i];
				x.m_light = lightArr[i];
				x.m_cacheEntry = lightArr[i]->m_textureArrayIndex;
				x.m_cmdbs = WeakArray<CommandBufferPtr>(
					alloc.newArray<CommandBufferPtr>(m_r->getThreadPool().getThreadsCount() * 6),
					m_r->getThreadPool().getThreadsCount() * 6);
				x.m_firstBatchIdx = (batchIdx) % m_scratchSm.m_batchSize;
				batchIdx += 6;
			}
		}
	}
}

void ShadowMapping::setPreRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(ctx.m_shadowMapping.m_spotCasters.getSize() + ctx.m_shadowMapping.m_omniCasters.getSize() != 0)
	{
		cmdb->setTextureSurfaceBarrier(m_scratchSm.m_rt,
			TextureUsageBit::NONE,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
			TextureSurfaceInfo(0, 0, 0, 0));
	}
}

void ShadowMapping::setPostRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_SM);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(ctx.m_shadowMapping.m_spotCasters.getSize())
	{
		cmdb->setTextureSurfaceBarrier(m_spotTex,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));
	}

	for(RenderingContext::ShadowMapping::OmniCasterInfo& inf : ctx.m_shadowMapping.m_omniCasters)
	{
		for(U f = 0; f < 6; ++f)
		{
			cmdb->setTextureSurfaceBarrier(m_omniTexArray,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
				TextureUsageBit::SAMPLED_FRAGMENT,
				TextureSurfaceInfo(0, 0, f, inf.m_light->m_textureArrayIndex));
		}
	}

	cmdb->informTextureCurrentUsage(m_spotTex, TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->informTextureCurrentUsage(m_omniTexArray, TextureUsageBit::SAMPLED_FRAGMENT);
}

} // end namespace anki
