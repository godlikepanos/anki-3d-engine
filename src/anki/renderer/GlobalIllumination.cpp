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
static Vec3 computeProbeCellPosition(U cellToRender, const GiProbeQueueElement& probe)
{
	UVec3 cellCoords;
	unflatten3dArrayIndex(probe.m_cellCounts.x(),
		probe.m_cellCounts.y(),
		probe.m_cellCounts.z(),
		cellToRender,
		cellCoords.x(),
		cellCoords.y(),
		cellCoords.z());

	const Vec3 cellSize = (probe.m_aabbMax - probe.m_aabbMin)
						  / Vec3(probe.m_cellCounts.x(), probe.m_cellCounts.y(), probe.m_cellCounts.z());
	const Vec3 halfCellSize = cellSize / 2.0f;
	const Vec3 cellPos = Vec3(cellCoords.x(), cellCoords.y(), cellCoords.z()) * cellSize + halfCellSize;

	return cellPos;
}

Error GlobalIllumination::initGBuffer(const ConfigSet& cfg)
{
	m_gbuffer.m_tileSize = cfg.getNumber("r.gi.gbufferResolution");

	// Create RT descriptions
	{
		RenderTargetDescription texinit = m_r->create2DRenderTargetDescription(
			m_gbuffer.m_tileSize * 6, m_gbuffer.m_tileSize, GBUFFER_COLOR_ATTACHMENT_PIXEL_FORMATS[0], "GI GBuffer");

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
	m_lightShading.m_tileSize = cfg.getNumber("r.gi.lightShadingResolution");

	// Init RT descr
	{
		m_lightShading.m_rtDescr = m_r->create2DRenderTargetDescription(m_lightShading.m_tileSize,
			m_lightShading.m_tileSize * 6,
			LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
			"GI LS");

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
	// TODO
	return Error::NONE;
}

void GlobalIllumination::populateRenderGraph(RenderingContext& rctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_GI);

#if ANKI_EXTRA_CHECKS
	m_ctx = {};
#endif

	RenderGraphDescription& rgraph = rctx.m_renderGraphDescr;

	// Prepare the probes
	GiProbeQueueElement* probeToUpdateThisFrame;
	U32 probeToUpdateThisFrameCacheEntryIdx;
	Vec4 eyeWorldPosition;
	prepareProbes(rctx, probeToUpdateThisFrame, probeToUpdateThisFrameCacheEntryIdx, eyeWorldPosition);

	if(ANKI_LIKELY(probeToUpdateThisFrame == nullptr))
	{
		// No probe to update
		return;
	}

	// GBuffer
	{
		// RTs
		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			m_ctx.m_gbufferColorRts[i] = rgraph.newRenderTarget(m_gbuffer.m_colorRtDescrs[i]);
		}
		m_ctx.m_gbufferDepthRt = rgraph.newRenderTarget(m_gbuffer.m_depthRtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI gbuff");
		pass.setFramebufferInfo(m_gbuffer.m_fbDescr, m_ctx.m_gbufferColorRts, m_ctx.m_gbufferDepthRt);
		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<GlobalIllumination*>(rgraphCtx.m_userData)->runGBufferInThread(rgraphCtx);
			},
			this,
			6);

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			pass.newDependency({m_ctx.m_gbufferColorRts[i], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		}

		TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
		pass.newDependency({m_ctx.m_gbufferDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
	}

	// Shadow pass. Optional
	if(probeToUpdateThisFrame->m_renderQueues[0]->m_directionalLight.m_uuid
		&& probeToUpdateThisFrame->m_renderQueues[0]->m_directionalLight.m_shadowCascadeCount > 0)
	{
		// Update light matrices
		for(U i = 0; i < 6; ++i)
		{
			ANKI_ASSERT(probeToUpdateThisFrame->m_renderQueues[i]->m_directionalLight.m_uuid
						&& probeToUpdateThisFrame->m_renderQueues[i]->m_directionalLight.m_shadowCascadeCount == 1);

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

			Mat4& lightMat = probeToUpdateThisFrame->m_renderQueues[i]->m_directionalLight.m_textureMatrices[0];
			lightMat = atlasMtx * lightMat;
		}

		// RT
		m_ctx.m_shadowsRt = rgraph.newRenderTarget(m_shadowMapping.m_rtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI SM");
		pass.setFramebufferInfo(m_shadowMapping.m_fbDescr, {}, m_ctx.m_shadowsRt);
		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<GlobalIllumination*>(rgraphCtx.m_userData)->runShadowmappingInThread(rgraphCtx);
			},
			this,
			6);

		TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
		pass.newDependency({m_ctx.m_shadowsRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
	}
	else
	{
		m_ctx.m_shadowsRt = {};
	}

	// Light shading pass
	RenderTargetHandle lightShadingRt;
	{
		// RT
		lightShadingRt = rgraph.newRenderTarget(m_lightShading.m_rtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI LS");
		pass.setFramebufferInfo(m_lightShading.m_fbDescr, {{lightShadingRt}}, {});
		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<GlobalIllumination*>(rgraphCtx.m_userData)->runLightShading(rgraphCtx);
			},
			this,
			0);

		pass.newDependency({lightShadingRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			pass.newDependency({m_ctx.m_gbufferColorRts[i], TextureUsageBit::SAMPLED_FRAGMENT});
		}
		pass.newDependency({m_ctx.m_gbufferDepthRt,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});

		if(m_ctx.m_shadowsRt.isValid())
		{
			pass.newDependency({m_ctx.m_shadowsRt, TextureUsageBit::SAMPLED_FRAGMENT});
		}
	}

	// Irradiance pass. First & 2nd bounce
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("GI IR");

		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<GlobalIllumination*>(rgraphCtx.m_userData)->runIrradiance(rgraphCtx);
			},
			this,
			0);

		pass.newDependency({lightShadingRt, TextureUsageBit::SAMPLED_COMPUTE});

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT - 1; ++i)
		{
			pass.newDependency({m_ctx.m_gbufferColorRts[i], TextureUsageBit::SAMPLED_COMPUTE});
		}

		for(U i = 0; i < 6; ++i)
		{
			pass.newDependency({m_cacheEntries[probeToUpdateThisFrameCacheEntryIdx].m_rtHandles[i],
				TextureUsageBit::IMAGE_COMPUTE_WRITE});
		}
	}
}

void GlobalIllumination::prepareProbes(RenderingContext& ctx,
	GiProbeQueueElement*& probeToUpdateThisFrame,
	U32& probeToUpdateThisFrameCacheEntryIdx,
	Vec4& cellWorldPosition)
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
	DynamicArray<GiProbeQueueElement> newListOfProbes;
	newListOfProbes.create(ctx.m_tempAllocator, ctx.m_renderQueue->m_giProbes.getSize());
	U newListOfProbeCount = 0;
	Bool foundProbeToUpdateNextFrame = false;
	for(U32 probeIdx = 0; probeIdx < ctx.m_renderQueue->m_giProbes.getSize(); ++probeIdx)
	{
		GiProbeQueueElement& probe = ctx.m_renderQueue->m_giProbes[probeIdx];

		// Find cache entry
		const U32 cacheEntryIdx = findBestCacheEntry(
			probe.m_uuid, m_r->getGlobalTimestamp(), m_cacheEntries, m_probeUuidToCacheEntryIdx, getAllocator());
		if(ANKI_UNLIKELY(cacheEntryIdx == MAX_U32))
		{
			// Failed
			ANKI_R_LOGW("There is not enough space in the indirect lighting atlas for more probes. "
						"Increase the r.gi.maxSimultaneousProbeCount or decrease the scene's probes");
			continue;
		}

		CacheEntry& entry = m_cacheEntries[cacheEntryIdx];

		const Bool cacheEntryDirty = entry.m_uuid != probe.m_uuid || entry.m_volumeSize != probe.m_cellCounts;
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

			probe.m_textureIndex = cacheEntryIdx;
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
			m_probeUuidToCacheEntryIdx.emplace(getAllocator(), probe.m_uuid, cacheEntryIdx);
		}

		// Update the cache entry
		entry.m_lastUsedTimestamp = m_r->getGlobalTimestamp();

		// Update the probe
		probe.m_textureIndex = cacheEntryIdx;

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
		probeToUpdateThisFrameCacheEntryIdx = cacheEntryIdx;
		probeToUpdateThisFrame = &newListOfProbes[newListOfProbeCount];

		// Compute the render position
		const U cellToRender = entry.m_renderedCells++;
		ANKI_ASSERT(cellToRender < probe.m_totalCellCount);
		cellWorldPosition = computeProbeCellPosition(cellToRender, probe).xyz0();

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
		GiProbeQueueElement* firstProbe;
		PtrSize probeCount, storage;
		newListOfProbes.moveAndReset(firstProbe, probeCount, storage);
		ctx.m_renderQueue->m_giProbes = WeakArray<GiProbeQueueElement>(firstProbe, newListOfProbeCount);
	}
	else
	{
		ctx.m_renderQueue->m_giProbes = WeakArray<GiProbeQueueElement>();
		newListOfProbes.destroy(ctx.m_tempAllocator);
	}
}

void GlobalIllumination::runGBufferInThread(RenderPassWorkContext& rgraphCtx) const
{
	ANKI_ASSERT(m_ctx.m_probe);
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	const GiProbeQueueElement& probe = *m_ctx.m_probe;
	const U faceIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	ANKI_ASSERT(faceIdx < 6);
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const U32 viewportX = faceIdx * m_gbuffer.m_tileSize;
	cmdb->setViewport(viewportX, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);
	cmdb->setScissor(viewportX, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);

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

	// Restore state
	cmdb->setScissor(0, 0, MAX_U32, MAX_U32);
}

void GlobalIllumination::runShadowmappingInThread(RenderPassWorkContext& rgraphCtx) const
{
	const U faceIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	ANKI_ASSERT(faceIdx < 6);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->setPolygonOffset(1.0f, 1.0f);

	ANKI_ASSERT(m_ctx.m_probe);
	ANKI_ASSERT(m_ctx.m_probe->m_renderQueues[faceIdx]);
	const RenderQueue& faceRenderQueue = *m_ctx.m_probe->m_renderQueues[faceIdx];
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

void GlobalIllumination::runLightShading(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	ANKI_ASSERT(m_ctx.m_probe);
	const GiProbeQueueElement& probe = *m_ctx.m_probe;

	for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
		const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

		const U rez = m_lightShading.m_tileSize;
		cmdb->setViewport(rez * faceIdx, 0, rez, rez);
		cmdb->setScissor(rez * faceIdx, 0, rez, rez);

		// Set common state for all lights
		// NOTE: Use nearest sampler because we don't want the result to sample the near tiles
		cmdb->bindSampler(0, 2, m_r->getSamplers().m_nearestNearestClamp);

		rgraphCtx.bindColorTexture(0, 3, m_ctx.m_gbufferColorRts[0]);
		rgraphCtx.bindColorTexture(0, 4, m_ctx.m_gbufferColorRts[1]);
		rgraphCtx.bindColorTexture(0, 5, m_ctx.m_gbufferColorRts[2]);

		rgraphCtx.bindTexture(0, 6, m_ctx.m_gbufferDepthRt, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

		// Get shadowmap info
		const Bool hasDirLight = probe.m_renderQueues[0]->m_directionalLight.m_uuid;
		if(hasDirLight)
		{
			ANKI_ASSERT(m_ctx.m_shadowsRt.isValid());

			cmdb->bindSampler(0, 7, m_shadowMapping.m_shadowSampler);

			rgraphCtx.bindTexture(0, 8, m_ctx.m_shadowsRt, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
		}

		m_lightShading.m_deferred.drawLights(rqueue.m_viewProjectionMatrix,
			rqueue.m_viewProjectionMatrix.getInverse(),
			rqueue.m_cameraTransform.getTranslationPart(),
			UVec4(0, 0, m_lightShading.m_tileSize, m_lightShading.m_tileSize),
			Vec2(faceIdx * (1.0f / 6.0f), 0.0f),
			Vec2((faceIdx + 1) * (1.0f / 6.0f), 1.0f),
			probe.m_renderQueues[faceIdx]->m_cameraNear,
			probe.m_renderQueues[faceIdx]->m_cameraFar,
			(hasDirLight) ? &probe.m_renderQueues[faceIdx]->m_directionalLight : nullptr,
			rqueue.m_pointLights,
			rqueue.m_spotLights,
			cmdb);
	}

	// Restore state
	cmdb->setScissor(0, 0, MAX_U32, MAX_U32);
}

void GlobalIllumination::runIrradiance(RenderPassWorkContext& rgraphCtx) const
{
	// TODO
}

} // end namespace anki
