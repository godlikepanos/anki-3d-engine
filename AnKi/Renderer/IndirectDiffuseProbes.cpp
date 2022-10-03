// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Collision/Functions.h>

namespace anki {

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

class IndirectDiffuseProbes::InternalContext
{
public:
	IndirectDiffuseProbes* m_gi ANKI_DEBUG_CODE(= numberToPtr<IndirectDiffuseProbes*>(1));
	RenderingContext* m_ctx ANKI_DEBUG_CODE(= numberToPtr<RenderingContext*>(1));

	GlobalIlluminationProbeQueueElement*
		m_probeToUpdateThisFrame ANKI_DEBUG_CODE(= numberToPtr<GlobalIlluminationProbeQueueElement*>(1));
	UVec3 m_cellOfTheProbeToUpdateThisFrame ANKI_DEBUG_CODE(= UVec3(kMaxU32));

	Array<RenderTargetHandle, kGBufferColorRenderTargetCount> m_gbufferColorRts;
	RenderTargetHandle m_gbufferDepthRt;
	RenderTargetHandle m_shadowsRt;
	RenderTargetHandle m_lightShadingRt;
	WeakArray<RenderTargetHandle> m_irradianceProbeRts;

	U32 m_gbufferDrawcallCount ANKI_DEBUG_CODE(= kMaxU32);
	U32 m_smDrawcallCount ANKI_DEBUG_CODE(= kMaxU32);

	static void foo()
	{
		static_assert(std::is_trivially_destructible<InternalContext>::value, "See file");
	}
};

IndirectDiffuseProbes::~IndirectDiffuseProbes()
{
	m_cacheEntries.destroy(getMemoryPool());
	m_probeUuidToCacheEntryIdx.destroy(getMemoryPool());
}

const RenderTargetHandle&
IndirectDiffuseProbes::getVolumeRenderTarget(const GlobalIlluminationProbeQueueElement& probe) const
{
	ANKI_ASSERT(m_giCtx);
	ANKI_ASSERT(&probe >= &m_giCtx->m_ctx->m_renderQueue->m_giProbes.getFront()
				&& &probe <= &m_giCtx->m_ctx->m_renderQueue->m_giProbes.getBack());
	const U32 idx = U32(&probe - &m_giCtx->m_ctx->m_renderQueue->m_giProbes.getFront());
	return m_giCtx->m_irradianceProbeRts[idx];
}

void IndirectDiffuseProbes::setRenderGraphDependencies(const RenderingContext& ctx, RenderPassDescriptionBase& pass,
													   TextureUsageBit usage) const
{
	for(U32 idx = 0; idx < ctx.m_renderQueue->m_giProbes.getSize(); ++idx)
	{
		pass.newTextureDependency(getVolumeRenderTarget(ctx.m_renderQueue->m_giProbes[idx]), usage);
	}
}

void IndirectDiffuseProbes::bindVolumeTextures(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx, U32 set,
											   U32 binding) const
{
	for(U32 idx = 0; idx < kMaxVisibleGlobalIlluminationProbes; ++idx)
	{
		if(idx < ctx.m_renderQueue->m_giProbes.getSize())
		{
			rgraphCtx.bindColorTexture(set, binding, getVolumeRenderTarget(ctx.m_renderQueue->m_giProbes[idx]), idx);
		}
		else
		{
			rgraphCtx.m_commandBuffer->bindTexture(set, binding, m_r->getDummyTextureView3d(), idx);
		}
	}
}

Error IndirectDiffuseProbes::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize global illumination");
	}

	return err;
}

Error IndirectDiffuseProbes::initInternal()
{
	m_tileSize = getConfig().getRIndirectDiffuseProbeTileResolution();
	m_cacheEntries.create(getMemoryPool(), getConfig().getRIndirectDiffuseProbeMaxCachedProbes());
	m_maxVisibleProbes = getConfig().getRIndirectDiffuseProbeMaxVisibleProbes();
	ANKI_ASSERT(m_maxVisibleProbes <= kMaxVisibleGlobalIlluminationProbes);
	ANKI_ASSERT(m_cacheEntries.getSize() >= m_maxVisibleProbes);

	ANKI_CHECK(initGBuffer());
	ANKI_CHECK(initLightShading());
	ANKI_CHECK(initShadowMapping());
	ANKI_CHECK(initIrradiance());

	return Error::kNone;
}

Error IndirectDiffuseProbes::initGBuffer()
{
	// Create RT descriptions
	{
		RenderTargetDescription texinit = m_r->create2DRenderTargetDescription(
			m_tileSize * 6, m_tileSize, kGBufferColorRenderTargetFormats[0], "GI GBuffer");

		// Create color RT descriptions
		for(U32 i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			texinit.m_format = kGBufferColorRenderTargetFormats[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(
				StringRaii(&getMemoryPool()).sprintf("GI GBuff Col #%u", i).toCString());
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_format = m_r->getDepthNoStencilFormat();
		texinit.setName("GI GBuff Depth");
		m_gbuffer.m_depthRtDescr = texinit;
		m_gbuffer.m_depthRtDescr.bake();
	}

	// Create FB descr
	{
		m_gbuffer.m_fbDescr.m_colorAttachmentCount = kGBufferColorRenderTargetCount;

		for(U j = 0; j < kGBufferColorRenderTargetCount; ++j)
		{
			m_gbuffer.m_fbDescr.m_colorAttachments[j].m_loadOperation = AttachmentLoadOperation::kClear;
		}

		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kClear;
		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;

		m_gbuffer.m_fbDescr.bake();
	}

	return Error::kNone;
}

Error IndirectDiffuseProbes::initShadowMapping()
{
	const U32 resolution = getConfig().getRIndirectDiffuseProbeShadowMapResolution();
	ANKI_ASSERT(resolution > 8);

	// RT descr
	m_shadowMapping.m_rtDescr =
		m_r->create2DRenderTargetDescription(resolution * 6, resolution, m_r->getDepthNoStencilFormat(), "GI SM");
	m_shadowMapping.m_rtDescr.bake();

	// FB descr
	m_shadowMapping.m_fbDescr.m_colorAttachmentCount = 0;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kClear;
	m_shadowMapping.m_fbDescr.bake();

	return Error::kNone;
}

Error IndirectDiffuseProbes::initLightShading()
{
	// Init RT descr
	{
		m_lightShading.m_rtDescr =
			m_r->create2DRenderTargetDescription(m_tileSize * 6, m_tileSize, m_r->getHdrFormat(), "GI LS");
		m_lightShading.m_rtDescr.bake();
	}

	// Create FB descr
	{
		m_lightShading.m_fbDescr.m_colorAttachmentCount = 1;
		m_lightShading.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kClear;
		m_lightShading.m_fbDescr.bake();
	}

	// Init deferred
	ANKI_CHECK(m_lightShading.m_deferred.init());

	return Error::kNone;
}

Error IndirectDiffuseProbes::initIrradiance()
{
	ANKI_CHECK(
		m_r->getResourceManager().loadResource("ShaderBinaries/IrradianceDice.ankiprogbin", m_irradiance.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_irradiance.m_prog);
	variantInitInfo.addMutation("WORKGROUP_SIZE_XY", m_tileSize);
	variantInitInfo.addMutation("LIGHT_SHADING_TEX", 0);
	variantInitInfo.addMutation("STORE_LOCATION", 0);
	variantInitInfo.addMutation("SECOND_BOUNCE", 1);

	const ShaderProgramResourceVariant* variant;
	m_irradiance.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_irradiance.m_grProg = variant->getProgram();

	return Error::kNone;
}

void IndirectDiffuseProbes::populateRenderGraph(RenderingContext& rctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	InternalContext* giCtx = newInstance<InternalContext>(*rctx.m_tempPool);
	m_giCtx = giCtx;
	giCtx->m_gi = this;
	giCtx->m_ctx = &rctx;
	RenderGraphDescription& rgraph = rctx.m_renderGraphDescr;

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
		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			giCtx->m_gbufferColorRts[i] = rgraph.newRenderTarget(m_gbuffer.m_colorRtDescrs[i]);
		}
		giCtx->m_gbufferDepthRt = rgraph.newRenderTarget(m_gbuffer.m_depthRtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI gbuff");
		pass.setFramebufferInfo(m_gbuffer.m_fbDescr, giCtx->m_gbufferColorRts, giCtx->m_gbufferDepthRt);
		pass.setWork(gbufferTaskCount, [this, giCtx](RenderPassWorkContext& rgraphCtx) {
			runGBufferInThread(rgraphCtx, *giCtx);
		});

		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			pass.newTextureDependency(giCtx->m_gbufferColorRts[i], TextureUsageBit::kFramebufferWrite);
		}

		TextureSubresourceInfo subresource(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(giCtx->m_gbufferDepthRt, TextureUsageBit::kAllFramebuffer, subresource);
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
		pass.setWork(smTaskCount, [this, giCtx](RenderPassWorkContext& rgraphCtx) {
			runShadowmappingInThread(rgraphCtx, *giCtx);
		});

		TextureSubresourceInfo subresource(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(giCtx->m_shadowsRt, TextureUsageBit::kAllFramebuffer, subresource);
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
		pass.setFramebufferInfo(m_lightShading.m_fbDescr, {giCtx->m_lightShadingRt});
		pass.setWork(1, [this, giCtx](RenderPassWorkContext& rgraphCtx) {
			runLightShading(rgraphCtx, *giCtx);
		});

		pass.newTextureDependency(giCtx->m_lightShadingRt, TextureUsageBit::kFramebufferWrite);

		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			pass.newTextureDependency(giCtx->m_gbufferColorRts[i], TextureUsageBit::kSampledFragment);
		}
		pass.newTextureDependency(giCtx->m_gbufferDepthRt, TextureUsageBit::kSampledFragment,
								  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		if(giCtx->m_shadowsRt.isValid())
		{
			pass.newTextureDependency(giCtx->m_shadowsRt, TextureUsageBit::kSampledFragment);
		}
	}

	// Irradiance pass. First & 2nd bounce
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("GI IR");

		pass.setWork([this, giCtx](RenderPassWorkContext& rgraphCtx) {
			runIrradiance(rgraphCtx, *giCtx);
		});

		pass.newTextureDependency(giCtx->m_lightShadingRt, TextureUsageBit::kSampledCompute);

		for(U32 i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
		{
			pass.newTextureDependency(giCtx->m_gbufferColorRts[i], TextureUsageBit::kSampledCompute);
		}

		const U32 probeIdx = U32(giCtx->m_probeToUpdateThisFrame - &giCtx->m_ctx->m_renderQueue->m_giProbes.getFront());
		pass.newTextureDependency(giCtx->m_irradianceProbeRts[probeIdx], TextureUsageBit::kImageComputeWrite);
	}
}

void IndirectDiffuseProbes::prepareProbes(InternalContext& giCtx)
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
	newListOfProbes.create(*ctx.m_tempPool, ctx.m_renderQueue->m_giProbes.getSize());
	DynamicArray<RenderTargetHandle> volumeRts;
	volumeRts.create(*ctx.m_tempPool, ctx.m_renderQueue->m_giProbes.getSize());
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
													 m_probeUuidToCacheEntryIdx, getMemoryPool());
		if(ANKI_UNLIKELY(cacheEntryIdx == kMaxU32))
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
				ctx.m_renderGraphDescr.importRenderTarget(entry.m_volumeTex, TextureUsageBit::kSampledFragment);
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
			m_probeUuidToCacheEntryIdx.emplace(getMemoryPool(), probe.m_uuid, cacheEntryIdx);
		}

		// Update the cache entry
		entry.m_lastUsedTimestamp = m_r->getGlobalTimestamp();

		// Init the cache entry textures
		const Bool shouldInitTextures = !entry.m_volumeTex.isCreated() || entry.m_volumeSize != probe.m_cellCounts;
		if(shouldInitTextures)
		{
			TextureInitInfo texInit("GiProbeVolume");
			texInit.m_type = TextureType::k3D;
			texInit.m_format = m_r->getHdrFormat();
			texInit.m_width = probe.m_cellCounts.x() * 6;
			texInit.m_height = probe.m_cellCounts.y();
			texInit.m_depth = probe.m_cellCounts.z();
			texInit.m_usage = TextureUsageBit::kAllCompute | TextureUsageBit::kAllSampled;

			entry.m_volumeTex = m_r->createAndClearRenderTarget(texInit, TextureUsageBit::kSampledFragment);
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
			ctx.m_renderGraphDescr.importRenderTarget(entry.m_volumeTex, TextureUsageBit::kSampledFragment);
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
		newListOfProbes.destroy(*ctx.m_tempPool);
		volumeRts.destroy(*ctx.m_tempPool);
	}
}

void IndirectDiffuseProbes::runGBufferInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const
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

			RenderableDrawerArguments args;
			args.m_viewMatrix = rqueue.m_viewMatrix;
			args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
			args.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
			args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
			args.m_sampler = m_r->getSamplers().m_trilinearRepeat;
			args.m_minLod = args.m_maxLod = kMaxLodCount - 1;

			m_r->getSceneDrawer().drawRange(RenderingTechnique::kGBuffer, args,
											rqueue.m_renderables.getBegin() + localStart,
											rqueue.m_renderables.getBegin() + localEnd, cmdb);
		}

		drawcallCount += faceDrawcallCount;
	}
	ANKI_ASSERT(giCtx.m_gbufferDrawcallCount == U32(drawcallCount));

	// It's secondary, no need to restore the state
}

void IndirectDiffuseProbes::runShadowmappingInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const
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

			RenderableDrawerArguments args;
			args.m_viewMatrix = cascadeRenderQueue.m_viewMatrix;
			args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
			args.m_viewProjectionMatrix = cascadeRenderQueue.m_viewProjectionMatrix;
			args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
			args.m_sampler = m_r->getSamplers().m_trilinearRepeatAniso;
			args.m_maxLod = args.m_minLod = kMaxLodCount - 1;

			m_r->getSceneDrawer().drawRange(RenderingTechnique::kShadow, args,
											cascadeRenderQueue.m_renderables.getBegin() + localStart,
											cascadeRenderQueue.m_renderables.getBegin() + localEnd, cmdb);
		}
	}

	// It's secondary, no need to restore the state
}

void IndirectDiffuseProbes::runLightShading(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx)
{
	ANKI_TRACE_SCOPED_EVENT(R_GI);

	ANKI_ASSERT(giCtx.m_probeToUpdateThisFrame);
	const GlobalIlluminationProbeQueueElement& probe = *giCtx.m_probeToUpdateThisFrame;

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
		const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

		const U32 rez = m_tileSize;
		cmdb->setScissor(rez * faceIdx, 0, rez, rez);
		cmdb->setViewport(rez * faceIdx, 0, rez, rez);

		// Draw light shading
		TraditionalDeferredLightShadingDrawInfo dsInfo;
		dsInfo.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
		dsInfo.m_invViewProjectionMatrix = rqueue.m_viewProjectionMatrix.getInverse();
		dsInfo.m_cameraPosWSpace = rqueue.m_cameraTransform.getTranslationPart().xyz1();
		dsInfo.m_viewport = UVec4(faceIdx * m_tileSize, 0, m_tileSize, m_tileSize);
		dsInfo.m_gbufferTexCoordsScale = Vec2(1.0f / F32(m_tileSize * 6), 1.0f / F32(m_tileSize));
		dsInfo.m_gbufferTexCoordsBias = Vec2(0.0f, 0.0f);
		dsInfo.m_lightbufferTexCoordsBias = Vec2(-F32(faceIdx), 0.0f);
		dsInfo.m_lightbufferTexCoordsScale = Vec2(1.0f / F32(m_tileSize), 1.0f / F32(m_tileSize));
		dsInfo.m_cameraNear = rqueue.m_cameraNear;
		dsInfo.m_cameraFar = rqueue.m_cameraFar;
		dsInfo.m_directionalLight = (rqueue.m_directionalLight.isEnabled()) ? &rqueue.m_directionalLight : nullptr;
		dsInfo.m_pointLights = rqueue.m_pointLights;
		dsInfo.m_spotLights = rqueue.m_spotLights;
		dsInfo.m_commandBuffer = cmdb;
		dsInfo.m_gbufferRenderTargets[0] = giCtx.m_gbufferColorRts[0];
		dsInfo.m_gbufferRenderTargets[1] = giCtx.m_gbufferColorRts[1];
		dsInfo.m_gbufferRenderTargets[2] = giCtx.m_gbufferColorRts[2];
		dsInfo.m_gbufferDepthRenderTarget = giCtx.m_gbufferDepthRt;
		if(dsInfo.m_directionalLight && dsInfo.m_directionalLight->hasShadow())
		{
			dsInfo.m_directionalLightShadowmapRenderTarget = giCtx.m_shadowsRt;
		}
		dsInfo.m_renderpassContext = &rgraphCtx;
		dsInfo.m_skybox = &giCtx.m_ctx->m_renderQueue->m_skybox;

		m_lightShading.m_deferred.drawLights(dsInfo);
	}
}

void IndirectDiffuseProbes::runIrradiance(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx)
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

	for(U32 i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
	{
		rgraphCtx.bindColorTexture(0, 2, giCtx.m_gbufferColorRts[i], i);
	}

	rgraphCtx.bindImage(0, 3, giCtx.m_irradianceProbeRts[probeIdx], TextureSubresourceInfo());

	class
	{
	public:
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
