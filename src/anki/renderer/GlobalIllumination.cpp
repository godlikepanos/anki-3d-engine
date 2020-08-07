// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/GlobalIllumination.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/ConfigSet.h>
#include <anki/util/Tracer.h>
#include <anki/collision/Aabb.h>
#include <anki/collision/Functions.h>

namespace anki
{

/// Given a cell index compute its world position.
static Vec3 computeProbeCellPosition(U32 cellIdx, const GlobalIlluminationProbeQueueElement& probe)
{
	ANKI_ASSERT(cellIdx < probe.m_totalCellCount);

	UVec3 cellCoords;
	unflatten3dArrayIndex(probe.m_cellCounts.z(), probe.m_cellCounts.y(), probe.m_cellCounts.x(), cellIdx,
						  cellCoords.z(), cellCoords.y(), cellCoords.x());

	const Vec3 halfCellSize = probe.m_cellSizes / 2.0f;
	const Vec3 cellPos = Vec3(cellCoords) * probe.m_cellSizes + halfCellSize + probe.m_aabbMin;
	ANKI_ASSERT(cellPos < probe.m_aabbMax);

	return cellPos;
}

class GlobalIllumination::InternalContext
{
public:
	GlobalIllumination* m_gi ANKI_DEBUG_CODE(= numberToPtr<GlobalIllumination*>(1));
	RenderingContext* m_ctx ANKI_DEBUG_CODE(= numberToPtr<RenderingContext*>(1));

	GlobalIlluminationProbeQueueElement*
		m_probeToUpdateThisFrame ANKI_DEBUG_CODE(= numberToPtr<GlobalIlluminationProbeQueueElement*>(1));
	UVec3 m_cellOfTheProbeToUpdateThisFrame ANKI_DEBUG_CODE(= UVec3(MAX_U32));

	Array<RenderTargetHandle, GBUFFER_COLOR_ATTACHMENT_COUNT> m_gbufferColorRts;
	RenderTargetHandle m_gbufferDepthRt;
	RenderTargetHandle m_shadowsRt;
	RenderTargetHandle m_lightShadingRt;
	WeakArray<RenderTargetHandle> m_irradianceProbeRts;

	U32 m_gbufferDrawcallCount ANKI_DEBUG_CODE(= MAX_U32);
	U32 m_smDrawcallCount ANKI_DEBUG_CODE(= MAX_U32);

	static void foo()
	{
		static_assert(std::is_trivially_destructible<InternalContext>::value, "See file");
	}
};

GlobalIllumination::~GlobalIllumination()
{
	m_cacheEntries.destroy(getAllocator());
	m_probeUuidToCacheEntryIdx.destroy(getAllocator());
}

const RenderTargetHandle&
GlobalIllumination::getVolumeRenderTarget(const GlobalIlluminationProbeQueueElement& probe) const
{
	ANKI_ASSERT(m_giCtx);
	ANKI_ASSERT(&probe >= &m_giCtx->m_ctx->m_renderQueue->m_giProbes.getFront()
				&& &probe <= &m_giCtx->m_ctx->m_renderQueue->m_giProbes.getBack());
	const U32 idx = U32(&probe - &m_giCtx->m_ctx->m_renderQueue->m_giProbes.getFront());
	return m_giCtx->m_irradianceProbeRts[idx];
}

void GlobalIllumination::setRenderGraphDependencies(RenderingContext& ctx, RenderPassDescriptionBase& pass,
													TextureUsageBit usage) const
{
	for(U32 idx = 0; idx < ctx.m_renderQueue->m_giProbes.getSize(); ++idx)
	{
		pass.newDependency({getVolumeRenderTarget(ctx.m_renderQueue->m_giProbes[idx]), usage});
	}
}

void GlobalIllumination::bindVolumeTextures(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx, U32 set,
											U32 binding) const
{
	for(U32 idx = 0; idx < MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES; ++idx)
	{
		if(idx < ctx.m_renderQueue->m_giProbes.getSize())
		{
			rgraphCtx.bindColorTexture(set, binding, getVolumeRenderTarget(ctx.m_renderQueue->m_giProbes[idx]), idx);
		}
		else
		{
			rgraphCtx.m_commandBuffer->bindTexture(set, binding, m_r->getDummyTextureView3d(),
												   TextureUsageBit::ALL_SAMPLED, idx);
		}
	}
}

Error GlobalIllumination::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing global illumination");

	const Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize global illumination");
	}

	return err;
}

Error GlobalIllumination::initInternal(const ConfigSet& cfg)
{
	m_tileSize = cfg.getNumberU32("r_giTileResolution");
	m_cacheEntries.create(getAllocator(), cfg.getNumberU32("r_giMaxCachedProbes"));
	m_maxVisibleProbes = cfg.getNumberU32("r_giMaxVisibleProbes");
	ANKI_ASSERT(m_maxVisibleProbes <= MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES);
	ANKI_ASSERT(m_cacheEntries.getSize() >= m_maxVisibleProbes);

	ANKI_CHECK(initGBuffer(cfg));
	ANKI_CHECK(initLightShading(cfg));
	ANKI_CHECK(initShadowMapping(cfg));
	ANKI_CHECK(initIrradiance(cfg));

	return Error::NONE;
}

Error GlobalIllumination::initGBuffer(const ConfigSet& cfg)
{
	// Create RT descriptions
	{
		RenderTargetDescription texinit = m_r->create2DRenderTargetDescription(
			m_tileSize * 6, m_tileSize, GBUFFER_COLOR_ATTACHMENT_PIXEL_FORMATS[0], "GI GBuffer");

		// Create color RT descriptions
		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			texinit.m_format = GBUFFER_COLOR_ATTACHMENT_PIXEL_FORMATS[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(StringAuto(getAllocator()).sprintf("GI GBuff Col #%u", i).toCString());
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_format = GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT;
		texinit.setName("GI GBuff Depth");
		m_gbuffer.m_depthRtDescr = texinit;
		m_gbuffer.m_depthRtDescr.bake();
	}

	// Create FB descr
	{
		m_gbuffer.m_fbDescr.m_colorAttachmentCount = GBUFFER_COLOR_ATTACHMENT_COUNT;

		for(U j = 0; j < GBUFFER_COLOR_ATTACHMENT_COUNT; ++j)
		{
			m_gbuffer.m_fbDescr.m_colorAttachments[j].m_loadOperation = AttachmentLoadOperation::CLEAR;
		}

		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;

		m_gbuffer.m_fbDescr.bake();
	}

	return Error::NONE;
}

Error GlobalIllumination::initShadowMapping(const ConfigSet& cfg)
{
	const U32 resolution = cfg.getNumberU32("r_giShadowMapResolution");
	ANKI_ASSERT(resolution > 8);

	// RT descr
	m_shadowMapping.m_rtDescr =
		m_r->create2DRenderTargetDescription(resolution * 6, resolution, Format::D32_SFLOAT, "GI SM");
	m_shadowMapping.m_rtDescr.bake();

	// FB descr
	m_shadowMapping.m_fbDescr.m_colorAttachmentCount = 0;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_shadowMapping.m_fbDescr.bake();

	// Shadow sampler
	{
		SamplerInitInfo inf;
		inf.m_compareOperation = CompareOperation::LESS_EQUAL;
		inf.m_addressing = SamplingAddressing::CLAMP;
		inf.m_mipmapFilter = SamplingFilter::BASE;
		inf.m_minMagFilter = SamplingFilter::LINEAR;
		m_shadowMapping.m_shadowSampler = getGrManager().newSampler(inf);
	}

	return Error::NONE;
}

Error GlobalIllumination::initLightShading(const ConfigSet& cfg)
{
	// Init RT descr
	{
		m_lightShading.m_rtDescr = m_r->create2DRenderTargetDescription(
			m_tileSize * 6, m_tileSize, LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT, "GI LS");
		m_lightShading.m_rtDescr.bake();
	}

	// Create FB descr
	{
		m_lightShading.m_fbDescr.m_colorAttachmentCount = 1;
		m_lightShading.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
		m_lightShading.m_fbDescr.bake();
	}

	// Init deferred
	ANKI_CHECK(m_lightShading.m_deferred.init());

	return Error::NONE;
}

Error GlobalIllumination::initIrradiance(const ConfigSet& cfg)
{
	ANKI_CHECK(m_r->getResourceManager().loadResource("shaders/IrradianceDice.ankiprog", m_irradiance.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_irradiance.m_prog);
	variantInitInfo.addMutation("WORKGROUP_SIZE_XY", m_tileSize);
	variantInitInfo.addMutation("LIGHT_SHADING_TEX", 0);
	variantInitInfo.addMutation("STORE_LOCATION", 0);
	variantInitInfo.addMutation("SECOND_BOUNCE", 1);

	const ShaderProgramResourceVariant* variant;
	m_irradiance.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_irradiance.m_grProg = variant->getProgram();

	return Error::NONE;
}

void GlobalIllumination::populateRenderGraph(RenderingContext& rctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	InternalContext* giCtx = rctx.m_tempAllocator.newInstance<InternalContext>();
	giCtx->m_gi = this;
	giCtx->m_ctx = &rctx;
	RenderGraphDescription& rgraph = rctx.m_renderGraphDescr;
	m_giCtx = giCtx;

	// Prepare the probes
	prepareProbes(*giCtx);
	const Bool haveProbeToRender = giCtx->m_probeToUpdateThisFrame != nullptr;
	if(!haveProbeToRender)
	{
		// Early exit
		return;
	}

	// Compute task counts for some of the passes
	U32 gbufferTaskCount, smTaskCount;
	{
		giCtx->m_gbufferDrawcallCount = 0;
		giCtx->m_smDrawcallCount = 0;
		for(const RenderQueue* rq : giCtx->m_probeToUpdateThisFrame->m_renderQueues)
		{
			ANKI_ASSERT(rq);
			giCtx->m_gbufferDrawcallCount += rq->m_renderables.getSize();

			if(rq->m_directionalLight.hasShadow())
			{
				giCtx->m_smDrawcallCount += rq->m_directionalLight.m_shadowRenderQueues[0]->m_renderables.getSize();
			}
		}

		gbufferTaskCount = computeNumberOfSecondLevelCommandBuffers(giCtx->m_gbufferDrawcallCount);
		smTaskCount = computeNumberOfSecondLevelCommandBuffers(giCtx->m_smDrawcallCount);
	}

	// GBuffer
	{
		// RTs
		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			giCtx->m_gbufferColorRts[i] = rgraph.newRenderTarget(m_gbuffer.m_colorRtDescrs[i]);
		}
		giCtx->m_gbufferDepthRt = rgraph.newRenderTarget(m_gbuffer.m_depthRtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI gbuff");
		pass.setFramebufferInfo(m_gbuffer.m_fbDescr, giCtx->m_gbufferColorRts, giCtx->m_gbufferDepthRt);
		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				InternalContext* giCtx = static_cast<InternalContext*>(rgraphCtx.m_userData);
				giCtx->m_gi->runGBufferInThread(rgraphCtx, *giCtx);
			},
			giCtx, gbufferTaskCount);

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			pass.newDependency({giCtx->m_gbufferColorRts[i], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		}

		TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
		pass.newDependency({giCtx->m_gbufferDepthRt, TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT, subresource});
	}

	// Shadow pass. Optional
	if(giCtx->m_probeToUpdateThisFrame->m_renderQueues[0]->m_directionalLight.m_uuid
	   && giCtx->m_probeToUpdateThisFrame->m_renderQueues[0]->m_directionalLight.m_shadowCascadeCount > 0)
	{
		// Update light matrices
		for(U i = 0; i < 6; ++i)
		{
			ANKI_ASSERT(giCtx->m_probeToUpdateThisFrame->m_renderQueues[i]->m_directionalLight.m_uuid
						&& giCtx->m_probeToUpdateThisFrame->m_renderQueues[i]->m_directionalLight.m_shadowCascadeCount
							   == 1);

			const F32 xScale = 1.0f / 6.0f;
			const F32 yScale = 1.0f;
			const F32 xOffset = F32(i) * (1.0f / 6.0f);
			const F32 yOffset = 0.0f;
			const Mat4 atlasMtx(xScale, 0.0f, 0.0f, xOffset, 0.0f, yScale, 0.0f, yOffset, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f);

			Mat4& lightMat =
				giCtx->m_probeToUpdateThisFrame->m_renderQueues[i]->m_directionalLight.m_textureMatrices[0];
			lightMat = atlasMtx * lightMat;
		}

		// RT
		giCtx->m_shadowsRt = rgraph.newRenderTarget(m_shadowMapping.m_rtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI SM");
		pass.setFramebufferInfo(m_shadowMapping.m_fbDescr, {}, giCtx->m_shadowsRt);
		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				InternalContext* giCtx = static_cast<InternalContext*>(rgraphCtx.m_userData);
				giCtx->m_gi->runShadowmappingInThread(rgraphCtx, *giCtx);
			},
			giCtx, smTaskCount);

		TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
		pass.newDependency({giCtx->m_shadowsRt, TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT, subresource});
	}
	else
	{
		giCtx->m_shadowsRt = {};
	}

	// Light shading pass
	{
		// RT
		giCtx->m_lightShadingRt = rgraph.newRenderTarget(m_lightShading.m_rtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI LS");
		pass.setFramebufferInfo(m_lightShading.m_fbDescr, {{giCtx->m_lightShadingRt}}, {});
		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				InternalContext* giCtx = static_cast<InternalContext*>(rgraphCtx.m_userData);
				giCtx->m_gi->runLightShading(rgraphCtx, *giCtx);
			},
			giCtx, 1);

		pass.newDependency({giCtx->m_lightShadingRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			pass.newDependency({giCtx->m_gbufferColorRts[i], TextureUsageBit::SAMPLED_FRAGMENT});
		}
		pass.newDependency({giCtx->m_gbufferDepthRt, TextureUsageBit::SAMPLED_FRAGMENT,
							TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});

		if(giCtx->m_shadowsRt.isValid())
		{
			pass.newDependency({giCtx->m_shadowsRt, TextureUsageBit::SAMPLED_FRAGMENT});
		}
	}

	// Irradiance pass. First & 2nd bounce
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("GI IR");

		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				InternalContext* giCtx = static_cast<InternalContext*>(rgraphCtx.m_userData);
				giCtx->m_gi->runIrradiance(rgraphCtx, *giCtx);
			},
			giCtx, 0);

		pass.newDependency({giCtx->m_lightShadingRt, TextureUsageBit::SAMPLED_COMPUTE});

		for(U32 i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT - 1; ++i)
		{
			pass.newDependency({giCtx->m_gbufferColorRts[i], TextureUsageBit::SAMPLED_COMPUTE});
		}

		const U32 probeIdx = U32(giCtx->m_probeToUpdateThisFrame - &giCtx->m_ctx->m_renderQueue->m_giProbes.getFront());
		pass.newDependency({giCtx->m_irradianceProbeRts[probeIdx], TextureUsageBit::IMAGE_COMPUTE_WRITE});
	}
}

void GlobalIllumination::prepareProbes(InternalContext& giCtx)
{
	RenderingContext& ctx = *giCtx.m_ctx;
	giCtx.m_probeToUpdateThisFrame = nullptr;

	if(ANKI_UNLIKELY(ctx.m_renderQueue->m_giProbes.getSize() == 0))
	{
		return;
	}

	// Iterate the probes and:
	// - Find a probe to update this frame
	// - Find a probe to update next frame
	// - Find the cache entries for each probe
	DynamicArray<GlobalIlluminationProbeQueueElement> newListOfProbes;
	newListOfProbes.create(ctx.m_tempAllocator, ctx.m_renderQueue->m_giProbes.getSize());
	DynamicArray<RenderTargetHandle> volumeRts;
	volumeRts.create(ctx.m_tempAllocator, ctx.m_renderQueue->m_giProbes.getSize());
	U32 newListOfProbeCount = 0;
	Bool foundProbeToUpdateNextFrame = false;
	for(U32 probeIdx = 0; probeIdx < ctx.m_renderQueue->m_giProbes.getSize(); ++probeIdx)
	{
		if(newListOfProbeCount + 1 >= m_maxVisibleProbes)
		{
			ANKI_R_LOGW("Can't have more that %u visible probes. Increase the r_giMaxVisibleProbes or (somehow) "
						"decrease the visible probes",
						m_maxVisibleProbes);
			break;
		}

		GlobalIlluminationProbeQueueElement& probe = ctx.m_renderQueue->m_giProbes[probeIdx];

		// Find cache entry
		const U32 cacheEntryIdx = findBestCacheEntry(probe.m_uuid, m_r->getGlobalTimestamp(), m_cacheEntries,
													 m_probeUuidToCacheEntryIdx, getAllocator());
		if(ANKI_UNLIKELY(cacheEntryIdx == MAX_U32))
		{
			// Failed
			ANKI_R_LOGW("There is not enough space in the indirect lighting atlas for more probes. "
						"Increase the r_giMaxCachedProbes or (somehow) decrease the visible probes");
			continue;
		}

		CacheEntry& entry = m_cacheEntries[cacheEntryIdx];

		const Bool cacheEntryDirty = entry.m_uuid != probe.m_uuid || entry.m_volumeSize != probe.m_cellCounts
									 || entry.m_probeAabbMin != probe.m_aabbMin
									 || entry.m_probeAabbMax != probe.m_aabbMax;
		const Bool needsUpdate = cacheEntryDirty || entry.m_renderedCells < probe.m_totalCellCount;

		if(ANKI_LIKELY(!needsUpdate))
		{
			// It's updated, early exit

			entry.m_lastUsedTimestamp = m_r->getGlobalTimestamp();
			volumeRts[newListOfProbeCount] =
				ctx.m_renderGraphDescr.importRenderTarget(entry.m_volumeTex, TextureUsageBit::SAMPLED_FRAGMENT);
			newListOfProbes[newListOfProbeCount++] = probe;
			continue;
		}

		// It needs update

		const Bool canUpdateThisFrame = giCtx.m_probeToUpdateThisFrame == nullptr && probe.m_renderQueues[0] != nullptr;
		const Bool canUpdateNextFrame = !foundProbeToUpdateNextFrame;

		if(!canUpdateThisFrame && canUpdateNextFrame)
		{
			// Probe will (possibly) be updated next frame, inform the probe

			foundProbeToUpdateNextFrame = true;

			const U32 cellToRender = (cacheEntryDirty) ? 0 : entry.m_renderedCells;
			const Vec3 cellPos = computeProbeCellPosition(cellToRender, probe);
			probe.m_feedbackCallback(true, probe.m_feedbackCallbackUserData, cellPos.xyz0());
			continue;
		}
		else if(!canUpdateThisFrame)
		{
			// Can't be updated this frame so remove it from the list
			continue;
		}

		// All good, can use this probe in this frame

		if(cacheEntryDirty)
		{
			entry.m_renderedCells = 0;
			entry.m_uuid = probe.m_uuid;
			entry.m_probeAabbMin = probe.m_aabbMin;
			entry.m_probeAabbMax = probe.m_aabbMax;
			entry.m_volumeSize = probe.m_cellCounts;
			m_probeUuidToCacheEntryIdx.emplace(getAllocator(), probe.m_uuid, cacheEntryIdx);
		}

		// Update the cache entry
		entry.m_lastUsedTimestamp = m_r->getGlobalTimestamp();

		// Init the cache entry textures
		const Bool shouldInitTextures = !entry.m_volumeTex.isCreated() || entry.m_volumeSize != probe.m_cellCounts;
		if(shouldInitTextures)
		{
			TextureInitInfo texInit;
			texInit.m_type = TextureType::_3D;
			texInit.m_format = Format::B10G11R11_UFLOAT_PACK32;
			texInit.m_width = probe.m_cellCounts.x() * 6;
			texInit.m_height = probe.m_cellCounts.y();
			texInit.m_depth = probe.m_cellCounts.z();
			texInit.m_usage = TextureUsageBit::ALL_COMPUTE | TextureUsageBit::ALL_SAMPLED;
			texInit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;

			entry.m_volumeTex = m_r->createAndClearRenderTarget(texInit);
		}

		// Compute the render position
		const U32 cellToRender = entry.m_renderedCells++;
		ANKI_ASSERT(cellToRender < probe.m_totalCellCount);
		unflatten3dArrayIndex(probe.m_cellCounts.z(), probe.m_cellCounts.y(), probe.m_cellCounts.x(), cellToRender,
							  giCtx.m_cellOfTheProbeToUpdateThisFrame.z(), giCtx.m_cellOfTheProbeToUpdateThisFrame.y(),
							  giCtx.m_cellOfTheProbeToUpdateThisFrame.x());

		// Inform probe about its next frame
		if(entry.m_renderedCells == probe.m_totalCellCount)
		{
			// Don't gather renderables next frame if it's done
			probe.m_feedbackCallback(false, probe.m_feedbackCallbackUserData, Vec4(0.0f));
		}
		else if(!foundProbeToUpdateNextFrame)
		{
			// Gather rendederables from the same probe next frame
			foundProbeToUpdateNextFrame = true;
			const Vec3 cellPos = computeProbeCellPosition(entry.m_renderedCells, probe);
			probe.m_feedbackCallback(true, probe.m_feedbackCallbackUserData, cellPos.xyz0());
		}

		// Push the probe to the new list
		giCtx.m_probeToUpdateThisFrame = &newListOfProbes[newListOfProbeCount];
		newListOfProbes[newListOfProbeCount] = probe;
		volumeRts[newListOfProbeCount] =
			ctx.m_renderGraphDescr.importRenderTarget(entry.m_volumeTex, TextureUsageBit::SAMPLED_FRAGMENT);
		++newListOfProbeCount;
	}

	// Replace the probe list in the queue
	if(newListOfProbeCount > 0)
	{
		GlobalIlluminationProbeQueueElement* firstProbe;
		U32 probeCount, storage;
		newListOfProbes.moveAndReset(firstProbe, probeCount, storage);
		ctx.m_renderQueue->m_giProbes = WeakArray<GlobalIlluminationProbeQueueElement>(firstProbe, newListOfProbeCount);

		RenderTargetHandle* firstRt;
		volumeRts.moveAndReset(firstRt, probeCount, storage);
		m_giCtx->m_irradianceProbeRts = WeakArray<RenderTargetHandle>(firstRt, newListOfProbeCount);
	}
	else
	{
		ctx.m_renderQueue->m_giProbes = WeakArray<GlobalIlluminationProbeQueueElement>();
		newListOfProbes.destroy(ctx.m_tempAllocator);
		volumeRts.destroy(ctx.m_tempAllocator);
	}
}

void GlobalIllumination::runGBufferInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const
{
	ANKI_ASSERT(giCtx.m_probeToUpdateThisFrame);
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const GlobalIlluminationProbeQueueElement& probe = *giCtx.m_probeToUpdateThisFrame;

	I32 start, end;
	U32 startu, endu;
	splitThreadedProblem(rgraphCtx.m_currentSecondLevelCommandBufferIndex, rgraphCtx.m_secondLevelCommandBufferCount,
						 giCtx.m_gbufferDrawcallCount, startu, endu);
	start = I32(startu);
	end = I32(endu);

	I32 drawcallCount = 0;
	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		const I32 faceDrawcallCount = I32(probe.m_renderQueues[faceIdx]->m_renderables.getSize());
		const I32 localStart = max(I32(0), start - drawcallCount);
		const I32 localEnd = min(faceDrawcallCount, end - drawcallCount);

		if(localStart < localEnd)
		{
			const U32 viewportX = faceIdx * m_tileSize;
			cmdb->setViewport(viewportX, 0, m_tileSize, m_tileSize);
			cmdb->setScissor(viewportX, 0, m_tileSize, m_tileSize);

			const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

			ANKI_ASSERT(localStart >= 0 && localEnd <= faceDrawcallCount);
			m_r->getSceneDrawer().drawRange(
				Pass::GB, rqueue.m_viewMatrix, rqueue.m_viewProjectionMatrix,
				Mat4::getIdentity(), // Don't care about prev mats since we don't care about velocity
				cmdb, m_r->getSamplers().m_trilinearRepeat, rqueue.m_renderables.getBegin() + localStart,
				rqueue.m_renderables.getBegin() + localEnd, MAX_LOD_COUNT - 1);
		}

		drawcallCount += faceDrawcallCount;
	}
	ANKI_ASSERT(giCtx.m_gbufferDrawcallCount == U32(drawcallCount));

	// It's secondary, no need to restore the state
}

void GlobalIllumination::runShadowmappingInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const
{
	ANKI_ASSERT(giCtx.m_probeToUpdateThisFrame);
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	const GlobalIlluminationProbeQueueElement& probe = *giCtx.m_probeToUpdateThisFrame;

	I32 start, end;
	U32 startu, endu;
	splitThreadedProblem(rgraphCtx.m_currentSecondLevelCommandBufferIndex, rgraphCtx.m_secondLevelCommandBufferCount,
						 giCtx.m_smDrawcallCount, startu, endu);
	start = I32(startu);
	end = I32(endu);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->setPolygonOffset(1.0f, 1.0f);

	I32 drawcallCount = 0;
	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
		const RenderQueue& faceRenderQueue = *probe.m_renderQueues[faceIdx];
		ANKI_ASSERT(faceRenderQueue.m_directionalLight.hasShadow());
		ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_shadowRenderQueues[0]);
		const RenderQueue& cascadeRenderQueue = *faceRenderQueue.m_directionalLight.m_shadowRenderQueues[0];

		const I32 faceDrawcallCount = I32(cascadeRenderQueue.m_renderables.getSize());
		const I32 localStart = max(I32(0), start - drawcallCount);
		const I32 localEnd = min(faceDrawcallCount, end - drawcallCount);

		if(localStart < localEnd)
		{
			const U32 rez = m_shadowMapping.m_rtDescr.m_height;
			cmdb->setViewport(rez * faceIdx, 0, rez, rez);
			cmdb->setScissor(rez * faceIdx, 0, rez, rez);

			ANKI_ASSERT(localStart >= 0 && localEnd <= faceDrawcallCount);
			m_r->getSceneDrawer().drawRange(Pass::SM, cascadeRenderQueue.m_viewMatrix,
											cascadeRenderQueue.m_viewProjectionMatrix,
											Mat4::getIdentity(), // Don't care about prev matrices here
											cmdb, m_r->getSamplers().m_trilinearRepeatAniso,
											cascadeRenderQueue.m_renderables.getBegin() + localStart,
											cascadeRenderQueue.m_renderables.getBegin() + localEnd, MAX_LOD_COUNT - 1);
		}
	}

	// It's secondary, no need to restore the state
}

void GlobalIllumination::runLightShading(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx)
{
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	ANKI_ASSERT(giCtx.m_probeToUpdateThisFrame);
	const GlobalIlluminationProbeQueueElement& probe = *giCtx.m_probeToUpdateThisFrame;

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	// Set common state for all lights
	{
		// NOTE: Use nearest sampler because we don't want the result to sample the near tiles
		cmdb->bindSampler(0, 2, m_r->getSamplers().m_nearestNearestClamp);

		rgraphCtx.bindColorTexture(0, 3, giCtx.m_gbufferColorRts[0]);
		rgraphCtx.bindColorTexture(0, 4, giCtx.m_gbufferColorRts[1]);
		rgraphCtx.bindColorTexture(0, 5, giCtx.m_gbufferColorRts[2]);

		rgraphCtx.bindTexture(0, 6, giCtx.m_gbufferDepthRt, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

		// Get shadowmap info
		if(probe.m_renderQueues[0]->m_directionalLight.isEnabled())
		{
			ANKI_ASSERT(giCtx.m_shadowsRt.isValid());

			cmdb->bindSampler(0, 7, m_shadowMapping.m_shadowSampler);

			rgraphCtx.bindTexture(0, 8, giCtx.m_shadowsRt, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
		}
	}

	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
		const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

		const U32 rez = m_tileSize;
		cmdb->setScissor(rez * faceIdx, 0, rez, rez);
		cmdb->setViewport(rez * faceIdx, 0, rez, rez);

		TraditionalDeferredLightShadingDrawInfo dsInfo;
		dsInfo.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
		dsInfo.m_invViewProjectionMatrix = rqueue.m_viewProjectionMatrix.getInverse();
		dsInfo.m_cameraPosWSpace = rqueue.m_cameraTransform.getTranslationPart();
		dsInfo.m_viewport = UVec4(faceIdx * m_tileSize, 0, m_tileSize, m_tileSize);
		dsInfo.m_gbufferTexCoordsScale = Vec2(1.0f / F32(m_tileSize * 6), 1.0f / F32(m_tileSize));
		dsInfo.m_gbufferTexCoordsBias = Vec2(0.0f, 0.0f);
		dsInfo.m_lightbufferTexCoordsScale = Vec2(1.0f / F32(m_tileSize), 1.0f / F32(m_tileSize));
		dsInfo.m_lightbufferTexCoordsBias = Vec2(-F32(faceIdx), 0.0f);
		dsInfo.m_cameraNear = probe.m_renderQueues[faceIdx]->m_cameraNear;
		dsInfo.m_cameraFar = probe.m_renderQueues[faceIdx]->m_cameraFar;
		dsInfo.m_directionalLight = (probe.m_renderQueues[faceIdx]->m_directionalLight.isEnabled())
										? &probe.m_renderQueues[faceIdx]->m_directionalLight
										: nullptr;
		dsInfo.m_pointLights = rqueue.m_pointLights;
		dsInfo.m_spotLights = rqueue.m_spotLights;
		dsInfo.m_commandBuffer = cmdb;
		m_lightShading.m_deferred.drawLights(dsInfo);
	}
}

void GlobalIllumination::runIrradiance(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx)
{
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	ANKI_ASSERT(giCtx.m_probeToUpdateThisFrame);
	const GlobalIlluminationProbeQueueElement& probe = *giCtx.m_probeToUpdateThisFrame;
	const U32 probeIdx = U32(&probe - &giCtx.m_ctx->m_renderQueue->m_giProbes.getFront());

	cmdb->bindShaderProgram(m_irradiance.m_grProg);

	// Bind resources
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
	rgraphCtx.bindColorTexture(0, 1, giCtx.m_lightShadingRt);

	for(U32 i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT - 1; ++i)
	{
		rgraphCtx.bindColorTexture(0, 2, giCtx.m_gbufferColorRts[i], i);
	}

	// Bind temporary memory
	allocateAndBindStorage<void*>(sizeof(Vec4) * 6 * m_tileSize * m_tileSize, cmdb, 0, 3);

	rgraphCtx.bindImage(0, 4, giCtx.m_irradianceProbeRts[probeIdx], TextureSubresourceInfo());

	struct
	{
		IVec3 m_volumeTexel;
		I32 m_nextTexelOffsetInU;
	} unis;

	unis.m_volumeTexel = IVec3(giCtx.m_cellOfTheProbeToUpdateThisFrame.x(), giCtx.m_cellOfTheProbeToUpdateThisFrame.y(),
							   giCtx.m_cellOfTheProbeToUpdateThisFrame.z());
	unis.m_nextTexelOffsetInU = probe.m_cellCounts.x();
	cmdb->setPushConstants(&unis, sizeof(unis));

	// Dispatch
	cmdb->dispatchCompute(1, 1, 1);
}

} // end namespace anki
