// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/GlobalIllumination.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/misc/ConfigSet.h>
#include <anki/core/Trace.h>

namespace anki
{

/// Given a cell index compute its world position.
static Vec3 computeProbeCellPosition(U cellIdx, const GlobalIlluminationProbeQueueElement& probe)
{
	ANKI_ASSERT(cellIdx < probe.m_totalCellCount);

	UVec3 cellCoords;
	unflatten3dArrayIndex(probe.m_cellCounts.x(),
		probe.m_cellCounts.y(),
		probe.m_cellCounts.z(),
		cellIdx,
		cellCoords.x(),
		cellCoords.y(),
		cellCoords.z());

	const Vec3 cellSize = (probe.m_aabbMax - probe.m_aabbMin)
						  / Vec3(probe.m_cellCounts.x(), probe.m_cellCounts.y(), probe.m_cellCounts.z());
	const Vec3 halfCellSize = cellSize / 2.0f;
	const Vec3 cellPos = Vec3(cellCoords.x(), cellCoords.y(), cellCoords.z()) * cellSize + halfCellSize;

	return cellPos;
}

class GlobalIllumination::InternalContext
{
public:
	GlobalIllumination* m_gi ANKI_DEBUG_CODE(= nullptr);
	RenderingContext* m_ctx ANKI_DEBUG_CODE(= nullptr);

	GlobalIlluminationProbeQueueElement* m_probe ANKI_DEBUG_CODE(= nullptr);
	UVec3 m_cell ANKI_DEBUG_CODE(= UVec3(MAX_U32));

	Array<RenderTargetHandle, GBUFFER_COLOR_ATTACHMENT_COUNT> m_gbufferColorRts;
	RenderTargetHandle m_gbufferDepthRt;
	RenderTargetHandle m_shadowsRt;
	RenderTargetHandle m_lightShadingRt;
};

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
	m_tileSize = cfg.getNumber("r.gi.tileResolution");
	m_cacheEntries.create(getAllocator(), cfg.getNumber("r.gi.maxSimultaneousProbeCount"));

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
	const U resolution = cfg.getNumber("r.gi.shadowMapResolution");
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
	ANKI_CHECK(m_r->getResourceManager().loadResource("shaders/IrradianceDice.glslp", m_irradiance.m_prog));

	ShaderProgramResourceConstantValueInitList<1> consts(m_irradiance.m_prog);
	consts.add("INPUT_TEXTURES_SIZE", U32(m_tileSize));

	const ShaderProgramResourceVariant* variant;
	m_irradiance.m_prog->getOrCreateVariant(consts.get(), variant);
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

	// Prepare the probes
	prepareProbes(rctx, giCtx->m_probe, giCtx->m_cell);

	if(ANKI_LIKELY(giCtx->m_probe == nullptr))
	{
		// No probe to update
		return;
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
			giCtx,
			6);

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			pass.newDependency({giCtx->m_gbufferColorRts[i], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		}

		TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
		pass.newDependency({giCtx->m_gbufferDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
	}

	// Shadow pass. Optional
	if(giCtx->m_probe->m_renderQueues[0]->m_directionalLight.m_uuid
		&& giCtx->m_probe->m_renderQueues[0]->m_directionalLight.m_shadowCascadeCount > 0)
	{
		// Update light matrices
		for(U i = 0; i < 6; ++i)
		{
			ANKI_ASSERT(giCtx->m_probe->m_renderQueues[i]->m_directionalLight.m_uuid
						&& giCtx->m_probe->m_renderQueues[i]->m_directionalLight.m_shadowCascadeCount == 1);

			const F32 xScale = 1.0f / 6.0f;
			const F32 yScale = 1.0f;
			const F32 xOffset = F32(i) * (1.0f / 6.0f);
			const F32 yOffset = 0.0f;
			const Mat4 atlasMtx(xScale,
				0.0f,
				0.0f,
				xOffset,
				0.0f,
				yScale,
				0.0f,
				yOffset,
				0.0f,
				0.0f,
				1.0f,
				0.0f,
				0.0f,
				0.0f,
				0.0f,
				1.0f);

			Mat4& lightMat = giCtx->m_probe->m_renderQueues[i]->m_directionalLight.m_textureMatrices[0];
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
			giCtx,
			6);

		TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
		pass.newDependency({giCtx->m_shadowsRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
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
			giCtx,
			6);

		pass.newDependency({giCtx->m_lightShadingRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			pass.newDependency({giCtx->m_gbufferColorRts[i], TextureUsageBit::SAMPLED_FRAGMENT});
		}
		pass.newDependency({giCtx->m_gbufferDepthRt,
			TextureUsageBit::SAMPLED_FRAGMENT,
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
			giCtx,
			0);

		pass.newDependency({giCtx->m_lightShadingRt, TextureUsageBit::SAMPLED_COMPUTE});

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT - 1; ++i)
		{
			pass.newDependency({giCtx->m_gbufferColorRts[i], TextureUsageBit::SAMPLED_COMPUTE});
		}

		for(U i = 0; i < 6; ++i)
		{
			pass.newDependency({m_cacheEntries[giCtx->m_probe->m_cacheEntryIndex].m_rtHandles[i],
				TextureUsageBit::IMAGE_COMPUTE_WRITE});
		}
	}
}

void GlobalIllumination::prepareProbes(RenderingContext& ctx,
	GlobalIlluminationProbeQueueElement*& probeToUpdateThisFrame,
	UVec3& probeToUpdateThisFrameCell)
{
	probeToUpdateThisFrame = nullptr;

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
	U newListOfProbeCount = 0;
	Bool foundProbeToUpdateNextFrame = false;
	for(U32 probeIdx = 0; probeIdx < ctx.m_renderQueue->m_giProbes.getSize(); ++probeIdx)
	{
		GlobalIlluminationProbeQueueElement& probe = ctx.m_renderQueue->m_giProbes[probeIdx];

		// Find cache entry
		const U32 cacheEntryIdx = findBestCacheEntry(
			probe.m_uuid, m_r->getGlobalTimestamp(), m_cacheEntries, m_probeUuidToCacheEntryIdx, getAllocator());
		if(ANKI_UNLIKELY(cacheEntryIdx == MAX_U32))
		{
			// Failed
			ANKI_R_LOGW("There is not enough space in the indirect lighting atlas for more probes. "
						"Increase the r.gi.maxSimultaneousProbeCount or (somehow) decrease the visible probes");
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
			for(U i = 0; i < 6; i++)
			{
				entry.m_rtHandles[i] = ctx.m_renderGraphDescr.importRenderTarget(
					entry.m_volumeTextures[i], TextureUsageBit::SAMPLED_FRAGMENT);
			}

			probe.m_cacheEntryIndex = cacheEntryIdx;
			newListOfProbes[newListOfProbeCount++] = probe;
			continue;
		}

		// It needs update

		const Bool canUpdateThisFrame = probeToUpdateThisFrame == nullptr && probe.m_renderQueues[0] != nullptr;
		const Bool canUpdateNextFrame = !foundProbeToUpdateNextFrame;

		if(!canUpdateThisFrame && canUpdateNextFrame)
		{
			// Probe will (possibly) be updated next frame, inform the probe

			foundProbeToUpdateNextFrame = true;

			const U cellToRender = (cacheEntryDirty) ? 0 : entry.m_renderedCells;
			const Vec3 cellPos = computeProbeCellPosition(cellToRender, probe);
			probe.m_feedbackCallback(true, probe.m_userData, cellPos.xyz0());
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
			m_probeUuidToCacheEntryIdx.emplace(getAllocator(), probe.m_uuid, cacheEntryIdx);
		}

		// Update the cache entry
		entry.m_lastUsedTimestamp = m_r->getGlobalTimestamp();

		// Update the probe
		probe.m_cacheEntryIndex = cacheEntryIdx;

		// Init the cache entry textures
		const Bool shouldInitTextures =
			!entry.m_volumeTextures[0].isCreated() || entry.m_volumeSize != probe.m_cellCounts;
		if(shouldInitTextures)
		{
			TextureInitInfo texInit;
			texInit.m_type = TextureType::_3D;
			texInit.m_format = Format::B10G11R11_UFLOAT_PACK32;
			texInit.m_width = probeToUpdateThisFrame->m_cellCounts.x();
			texInit.m_height = probeToUpdateThisFrame->m_cellCounts.y();
			texInit.m_depth = probeToUpdateThisFrame->m_cellCounts.z();
			texInit.m_usage = TextureUsageBit::ALL_COMPUTE | TextureUsageBit::SAMPLED_ALL;
			texInit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;

			for(U i = 0; i < 6; ++i)
			{
				entry.m_volumeTextures[i] = m_r->createAndClearRenderTarget(texInit);
			}
		}

		// Inform the caller
		probeToUpdateThisFrame = &newListOfProbes[newListOfProbeCount];

		// Compute the render position
		const U cellToRender = entry.m_renderedCells++;
		ANKI_ASSERT(cellToRender < probe.m_totalCellCount);
		unflatten3dArrayIndex(probe.m_cellCounts.x(),
			probe.m_cellCounts.y(),
			probe.m_cellCounts.z(),
			cellToRender,
			probeToUpdateThisFrameCell.x(),
			probeToUpdateThisFrameCell.y(),
			probeToUpdateThisFrameCell.z());

		// Inform probe about its next frame
		if(entry.m_renderedCells == probe.m_totalCellCount)
		{
			// Don't gather renderables next frame if it's done
			probe.m_feedbackCallback(false, probe.m_userData, Vec4(0.0f));
		}
		else if(!foundProbeToUpdateNextFrame)
		{
			// Gather rendederables from the same probe next frame
			foundProbeToUpdateNextFrame = true;
			const Vec3 cellPos = computeProbeCellPosition(entry.m_renderedCells, probe);
			probe.m_feedbackCallback(true, probe.m_userData, cellPos.xyz0());
		}

		// Push the probe to the new list
		newListOfProbes[newListOfProbeCount++] = probe;

		for(U i = 0; i < 6; i++)
		{
			entry.m_rtHandles[i] =
				ctx.m_renderGraphDescr.importRenderTarget(entry.m_volumeTextures[i], TextureUsageBit::SAMPLED_FRAGMENT);
		}
	}

	// Replace the probe list in the queue
	if(newListOfProbeCount > 0)
	{
		GlobalIlluminationProbeQueueElement* firstProbe;
		PtrSize probeCount, storage;
		newListOfProbes.moveAndReset(firstProbe, probeCount, storage);
		ctx.m_renderQueue->m_giProbes = WeakArray<GlobalIlluminationProbeQueueElement>(firstProbe, newListOfProbeCount);
	}
	else
	{
		ctx.m_renderQueue->m_giProbes = WeakArray<GlobalIlluminationProbeQueueElement>();
		newListOfProbes.destroy(ctx.m_tempAllocator);
	}
}

void GlobalIllumination::runGBufferInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const
{
	ANKI_ASSERT(giCtx.m_probe);
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	const GlobalIlluminationProbeQueueElement& probe = *giCtx.m_probe;
	const U faceIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	ANKI_ASSERT(faceIdx < 6);
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const U32 viewportX = faceIdx * m_tileSize;
	cmdb->setViewport(viewportX, 0, m_tileSize, m_tileSize);
	cmdb->setScissor(viewportX, 0, m_tileSize, m_tileSize);

	/// Draw
	ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
	const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

	if(!rqueue.m_renderables.isEmpty())
	{
		m_r->getSceneDrawer().drawRange(Pass::GB,
			rqueue.m_viewMatrix,
			rqueue.m_viewProjectionMatrix,
			Mat4::getIdentity(), // Don't care about prev mats since we don't care about velocity
			cmdb,
			m_r->getSamplers().m_trilinearRepeat,
			rqueue.m_renderables.getBegin(),
			rqueue.m_renderables.getEnd());
	}

	// It's secondary, no need to restore the state
}

void GlobalIllumination::runShadowmappingInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const
{
	const U faceIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	ANKI_ASSERT(faceIdx < 6);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->setPolygonOffset(1.0f, 1.0f);

	ANKI_ASSERT(giCtx.m_probe);
	ANKI_ASSERT(giCtx.m_probe->m_renderQueues[faceIdx]);
	const RenderQueue& faceRenderQueue = *giCtx.m_probe->m_renderQueues[faceIdx];
	ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_uuid != 0);
	ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_shadowCascadeCount == 1);

	ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_shadowRenderQueues[0]);
	const RenderQueue& cascadeRenderQueue = *faceRenderQueue.m_directionalLight.m_shadowRenderQueues[0];

	if(cascadeRenderQueue.m_renderables.getSize() != 0)
	{
		const U rez = m_shadowMapping.m_rtDescr.m_height;
		cmdb->setViewport(rez * faceIdx, 0, rez, rez);
		cmdb->setScissor(rez * faceIdx, 0, rez, rez);

		m_r->getSceneDrawer().drawRange(Pass::SM,
			cascadeRenderQueue.m_viewMatrix,
			cascadeRenderQueue.m_viewProjectionMatrix,
			Mat4::getIdentity(), // Don't care about prev matrices here
			cmdb,
			m_r->getSamplers().m_trilinearRepeatAniso,
			cascadeRenderQueue.m_renderables.getBegin(),
			cascadeRenderQueue.m_renderables.getEnd());
	}

	// It's secondary, no need to restore the state
}

void GlobalIllumination::runLightShading(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx)
{
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	ANKI_ASSERT(giCtx.m_probe);
	const GlobalIlluminationProbeQueueElement& probe = *giCtx.m_probe;
	const U faceIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	ANKI_ASSERT(faceIdx < 6);

	ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
	const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

	const U rez = m_tileSize;
	cmdb->setViewport(rez * faceIdx, 0, rez, rez);
	cmdb->setScissor(rez * faceIdx, 0, rez, rez);

	// Set common state for all lights
	// NOTE: Use nearest sampler because we don't want the result to sample the near tiles
	cmdb->bindSampler(0, 2, m_r->getSamplers().m_nearestNearestClamp);

	rgraphCtx.bindColorTexture(0, 3, giCtx.m_gbufferColorRts[0]);
	rgraphCtx.bindColorTexture(0, 4, giCtx.m_gbufferColorRts[1]);
	rgraphCtx.bindColorTexture(0, 5, giCtx.m_gbufferColorRts[2]);

	rgraphCtx.bindTexture(0, 6, giCtx.m_gbufferDepthRt, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	// Get shadowmap info
	const Bool hasDirLight = probe.m_renderQueues[0]->m_directionalLight.m_uuid;
	if(hasDirLight)
	{
		ANKI_ASSERT(giCtx.m_shadowsRt.isValid());

		cmdb->bindSampler(0, 7, m_shadowMapping.m_shadowSampler);

		rgraphCtx.bindTexture(0, 8, giCtx.m_shadowsRt, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	}

	m_lightShading.m_deferred.drawLights(rqueue.m_viewProjectionMatrix,
		rqueue.m_viewProjectionMatrix.getInverse(),
		rqueue.m_cameraTransform.getTranslationPart(),
		UVec4(0, 0, m_tileSize, m_tileSize),
		Vec2(faceIdx * (1.0f / 6.0f), 0.0f),
		Vec2((faceIdx + 1) * (1.0f / 6.0f), 1.0f),
		probe.m_renderQueues[faceIdx]->m_cameraNear,
		probe.m_renderQueues[faceIdx]->m_cameraFar,
		(hasDirLight) ? &probe.m_renderQueues[faceIdx]->m_directionalLight : nullptr,
		rqueue.m_pointLights,
		rqueue.m_spotLights,
		cmdb);
}

void GlobalIllumination::runIrradiance(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const
{
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	ANKI_ASSERT(giCtx.m_probe);
	const GlobalIlluminationProbeQueueElement& probe = *giCtx.m_probe;

	// Bind resources
	U binding = 0;
	cmdb->bindSampler(0, binding++, m_r->getSamplers().m_nearestNearestClamp);
	rgraphCtx.bindColorTexture(0, binding++, giCtx.m_lightShadingRt);

	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT - 1; ++i)
	{
		rgraphCtx.bindColorTexture(0, binding++, giCtx.m_gbufferColorRts[i]);
	}

	for(U i = 0; i < 6; ++i)
	{
		rgraphCtx.bindImage(
			0, binding, m_cacheEntries[probe.m_cacheEntryIndex].m_rtHandles[i], TextureSubresourceInfo(), i);
	}

	const IVec4 volumeTexel = IVec4(giCtx.m_cell.x(), giCtx.m_cell.y(), giCtx.m_cell.z(), 0);
	cmdb->setPushConstants(&volumeTexel, sizeof(volumeTexel));

	// Dispatch
	cmdb->dispatchCompute(1, 1, 1);
}

} // end namespace anki
