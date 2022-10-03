// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Shaders/Include/TraditionalDeferredShadingTypes.h>

namespace anki {

ProbeReflections::ProbeReflections(Renderer* r)
	: RendererObject(r)
	, m_lightShading(r)
{
}

ProbeReflections::~ProbeReflections()
{
	m_cacheEntries.destroy(getMemoryPool());
	m_probeUuidToCacheEntryIdx.destroy(getMemoryPool());
}

Error ProbeReflections::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize image reflections");
	}

	return err;
}

Error ProbeReflections::initInternal()
{
	// Init cache entries
	m_cacheEntries.create(getMemoryPool(), getConfig().getRProbeRefectionMaxCachedProbes());

	ANKI_CHECK(initGBuffer());
	ANKI_CHECK(initLightShading());
	ANKI_CHECK(initIrradiance());
	ANKI_CHECK(initIrradianceToRefl());
	ANKI_CHECK(initShadowMapping());

	// Load split sum integration LUT
	ANKI_CHECK(getResourceManager().loadResource("EngineAssets/IblDfg.png", m_integrationLut));

	SamplerInitInfo sinit;
	sinit.m_minMagFilter = SamplingFilter::kLinear;
	sinit.m_mipmapFilter = SamplingFilter::kBase;
	sinit.m_minLod = 0.0;
	sinit.m_maxLod = 1.0;
	sinit.m_addressing = SamplingAddressing::kClamp;
	m_integrationLutSampler = getGrManager().newSampler(sinit);

	return Error::kNone;
}

Error ProbeReflections::initGBuffer()
{
	m_gbuffer.m_tileSize = getConfig().getRProbeReflectionResolution();

	// Create RT descriptions
	{
		RenderTargetDescription texinit = m_r->create2DRenderTargetDescription(
			m_gbuffer.m_tileSize * 6, m_gbuffer.m_tileSize, kGBufferColorRenderTargetFormats[0], "CubeRefl GBuffer");

		// Create color RT descriptions
		for(U32 i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			texinit.m_format = kGBufferColorRenderTargetFormats[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(
				StringRaii(&getMemoryPool()).sprintf("CubeRefl GBuff Col #%u", i).toCString());
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_format = m_r->getDepthNoStencilFormat();
		texinit.setName("CubeRefl GBuff Depth");
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

Error ProbeReflections::initLightShading()
{
	m_lightShading.m_tileSize = getConfig().getRProbeReflectionResolution();
	m_lightShading.m_mipCount = computeMaxMipmapCount2d(m_lightShading.m_tileSize, m_lightShading.m_tileSize, 8);

	// Init cube arr
	{
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(
			m_lightShading.m_tileSize, m_lightShading.m_tileSize, m_r->getHdrFormat(),
			TextureUsageBit::kSampledFragment | TextureUsageBit::kSampledCompute | TextureUsageBit::kImageComputeRead
				| TextureUsageBit::kImageComputeWrite | TextureUsageBit::kAllFramebuffer
				| TextureUsageBit::kGenerateMipmaps,
			"CubeRefl refl");
		texinit.m_mipmapCount = U8(m_lightShading.m_mipCount);
		texinit.m_type = TextureType::kCubeArray;
		texinit.m_layerCount = m_cacheEntries.getSize();

		m_lightShading.m_cubeArr = m_r->createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment);
	}

	// Init deferred
	ANKI_CHECK(m_lightShading.m_deferred.init());

	return Error::kNone;
}

Error ProbeReflections::initIrradiance()
{
	m_irradiance.m_workgroupSize = getConfig().getRProbeReflectionIrradianceResolution();

	// Create prog
	{
		ANKI_CHECK(
			m_r->getResourceManager().loadResource("ShaderBinaries/IrradianceDice.ankiprogbin", m_irradiance.m_prog));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_irradiance.m_prog);

		variantInitInfo.addMutation("WORKGROUP_SIZE_XY", U32(m_irradiance.m_workgroupSize));
		variantInitInfo.addMutation("LIGHT_SHADING_TEX", 1);
		variantInitInfo.addMutation("STORE_LOCATION", 1);
		variantInitInfo.addMutation("SECOND_BOUNCE", 0);

		const ShaderProgramResourceVariant* variant;
		m_irradiance.m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_irradiance.m_grProg = variant->getProgram();
	}

	// Create buff
	{
		BufferInitInfo init;
		init.m_usage = BufferUsageBit::kAllStorage;
		init.m_size = 6 * sizeof(Vec4);
		m_irradiance.m_diceValuesBuff = getGrManager().newBuffer(init);
	}

	return Error::kNone;
}

Error ProbeReflections::initIrradianceToRefl()
{
	// Create program
	ANKI_CHECK(m_r->getResourceManager().loadResource("ShaderBinaries/ApplyIrradianceToReflection.ankiprogbin",
													  m_irradianceToRefl.m_prog));

	const ShaderProgramResourceVariant* variant;
	m_irradianceToRefl.m_prog->getOrCreateVariant(ShaderProgramResourceVariantInitInfo(m_irradianceToRefl.m_prog),
												  variant);
	m_irradianceToRefl.m_grProg = variant->getProgram();

	return Error::kNone;
}

Error ProbeReflections::initShadowMapping()
{
	const U32 resolution = getConfig().getRProbeReflectionShadowMapResolution();
	ANKI_ASSERT(resolution > 8);

	// RT descr
	m_shadowMapping.m_rtDescr =
		m_r->create2DRenderTargetDescription(resolution * 6, resolution, m_r->getDepthNoStencilFormat(), "CubeRefl SM");
	m_shadowMapping.m_rtDescr.bake();

	// FB descr
	m_shadowMapping.m_fbDescr.m_colorAttachmentCount = 0;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kClear;
	m_shadowMapping.m_fbDescr.bake();

	return Error::kNone;
}

void ProbeReflections::initCacheEntry(U32 cacheEntryIdx)
{
	CacheEntry& cacheEntry = m_cacheEntries[cacheEntryIdx];

	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		// Light pass FB
		FramebufferDescription& fbDescr = cacheEntry.m_lightShadingFbDescrs[faceIdx];
		ANKI_ASSERT(!fbDescr.isBacked());
		fbDescr.m_colorAttachmentCount = 1;
		fbDescr.m_colorAttachments[0].m_surface.m_layer = cacheEntryIdx;
		fbDescr.m_colorAttachments[0].m_surface.m_face = faceIdx;
		fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kClear;
		fbDescr.bake();
	}
}

void ProbeReflections::prepareProbes(RenderingContext& ctx, ReflectionProbeQueueElement*& probeToUpdateThisFrame,
									 U32& probeToUpdateThisFrameCacheEntryIdx)
{
	probeToUpdateThisFrame = nullptr;
	probeToUpdateThisFrameCacheEntryIdx = kMaxU32;

	if(ANKI_UNLIKELY(ctx.m_renderQueue->m_reflectionProbes.getSize() == 0))
	{
		return;
	}

	// Iterate the probes and:
	// - Find a probe to update this frame
	// - Find a probe to update next frame
	// - Find the cache entries for each probe
	DynamicArray<ReflectionProbeQueueElement> newListOfProbes;
	newListOfProbes.create(*ctx.m_tempPool, ctx.m_renderQueue->m_reflectionProbes.getSize());
	U32 newListOfProbeCount = 0;
	Bool foundProbeToUpdateNextFrame = false;
	for(U32 probeIdx = 0; probeIdx < ctx.m_renderQueue->m_reflectionProbes.getSize(); ++probeIdx)
	{
		ReflectionProbeQueueElement& probe = ctx.m_renderQueue->m_reflectionProbes[probeIdx];

		// Find cache entry
		const U32 cacheEntryIdx = findBestCacheEntry(probe.m_uuid, m_r->getGlobalTimestamp(), m_cacheEntries,
													 m_probeUuidToCacheEntryIdx, getMemoryPool());
		if(ANKI_UNLIKELY(cacheEntryIdx == kMaxU32))
		{
			// Failed
			ANKI_R_LOGW("There is not enough space in the indirect lighting atlas for more probes. "
						"Increase the r_probeRefectionlMaxSimultaneousProbeCount or decrease the scene's probes");
			continue;
		}

		const Bool probeFoundInCache = m_cacheEntries[cacheEntryIdx].m_uuid == probe.m_uuid;

		// Check if we _should_ and _can_ update the probe
		const Bool needsUpdate = !probeFoundInCache;
		if(ANKI_UNLIKELY(needsUpdate))
		{
			const Bool canUpdateThisFrame = probeToUpdateThisFrame == nullptr && probe.m_renderQueues[0] != nullptr;
			const Bool canUpdateNextFrame = !foundProbeToUpdateNextFrame;

			if(!canUpdateThisFrame && canUpdateNextFrame)
			{
				// Probe will be updated next frame
				foundProbeToUpdateNextFrame = true;
				probe.m_feedbackCallback(true, probe.m_feedbackCallbackUserData);
				continue;
			}
			else if(!canUpdateThisFrame)
			{
				// Can't be updated this frame so remove it from the list
				continue;
			}
			else
			{
				// Can be updated this frame so continue with it
				probeToUpdateThisFrameCacheEntryIdx = cacheEntryIdx;
				probeToUpdateThisFrame = &newListOfProbes[newListOfProbeCount];
			}
		}

		// All good, can use this probe in this frame

		// Update the cache entry
		m_cacheEntries[cacheEntryIdx].m_uuid = probe.m_uuid;
		m_cacheEntries[cacheEntryIdx].m_lastUsedTimestamp = m_r->getGlobalTimestamp();

		// Update the probe
		probe.m_textureArrayIndex = cacheEntryIdx;

		// Push the probe to the new list
		newListOfProbes[newListOfProbeCount++] = probe;

		// Update cache map
		if(!probeFoundInCache)
		{
			m_probeUuidToCacheEntryIdx.emplace(getMemoryPool(), probe.m_uuid, cacheEntryIdx);
		}

		// Don't gather renderables next frame
		if(probe.m_renderQueues[0] != nullptr)
		{
			probe.m_feedbackCallback(false, probe.m_feedbackCallbackUserData);
		}
	}

	// Replace the probe list in the queue
	if(newListOfProbeCount > 0)
	{
		ReflectionProbeQueueElement* firstProbe;
		U32 probeCount, storage;
		newListOfProbes.moveAndReset(firstProbe, probeCount, storage);
		ctx.m_renderQueue->m_reflectionProbes = WeakArray<ReflectionProbeQueueElement>(firstProbe, newListOfProbeCount);
	}
	else
	{
		ctx.m_renderQueue->m_reflectionProbes = WeakArray<ReflectionProbeQueueElement>();
		newListOfProbes.destroy(*ctx.m_tempPool);
	}
}

void ProbeReflections::runGBuffer(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_ctx.m_probe);
	ANKI_TRACE_SCOPED_EVENT(R_CUBE_REFL);
	const ReflectionProbeQueueElement& probe = *m_ctx.m_probe;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	I32 start, end;
	U32 startu, endu;
	splitThreadedProblem(rgraphCtx.m_currentSecondLevelCommandBufferIndex, rgraphCtx.m_secondLevelCommandBufferCount,
						 m_ctx.m_gbufferRenderableCount, startu, endu);
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
			const U32 viewportX = faceIdx * m_gbuffer.m_tileSize;
			cmdb->setViewport(viewportX, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);
			cmdb->setScissor(viewportX, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);

			const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];
			ANKI_ASSERT(localStart >= 0 && localEnd <= faceDrawcallCount);

			RenderableDrawerArguments args;
			args.m_viewMatrix = rqueue.m_viewMatrix;
			args.m_cameraTransform = rqueue.m_cameraTransform;
			args.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
			args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care about prev mats
			args.m_sampler = m_r->getSamplers().m_trilinearRepeat;
			args.m_minLod = args.m_maxLod = kMaxLodCount - 1;

			m_r->getSceneDrawer().drawRange(RenderingTechnique::kGBuffer, args,
											rqueue.m_renderables.getBegin() + localStart,
											rqueue.m_renderables.getBegin() + localEnd, cmdb);
		}
	}

	// Restore state
	cmdb->setScissor(0, 0, kMaxU32, kMaxU32);
}

void ProbeReflections::runLightShading(U32 faceIdx, const RenderingContext& rctx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(faceIdx <= 6);
	ANKI_TRACE_SCOPED_EVENT(R_CUBE_REFL);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	ANKI_ASSERT(m_ctx.m_probe);
	const ReflectionProbeQueueElement& probe = *m_ctx.m_probe;
	const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];
	const Bool hasDirLight = probe.m_renderQueues[0]->m_directionalLight.m_uuid;

	TraditionalDeferredLightShadingDrawInfo dsInfo;
	dsInfo.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
	dsInfo.m_invViewProjectionMatrix = rqueue.m_viewProjectionMatrix.getInverse();
	dsInfo.m_cameraPosWSpace = rqueue.m_cameraTransform.getTranslationPart().xyz1();
	dsInfo.m_viewport = UVec4(0, 0, m_lightShading.m_tileSize, m_lightShading.m_tileSize);
	dsInfo.m_gbufferTexCoordsScale =
		Vec2(1.0f / F32(m_lightShading.m_tileSize * 6), 1.0f / F32(m_lightShading.m_tileSize));
	dsInfo.m_gbufferTexCoordsBias = Vec2(F32(faceIdx) * (1.0f / 6.0f), 0.0f);
	dsInfo.m_lightbufferTexCoordsScale =
		Vec2(1.0f / F32(m_lightShading.m_tileSize), 1.0f / F32(m_lightShading.m_tileSize));
	dsInfo.m_lightbufferTexCoordsBias = Vec2(0.0f, 0.0f);
	dsInfo.m_cameraNear = probe.m_renderQueues[faceIdx]->m_cameraNear;
	dsInfo.m_cameraFar = probe.m_renderQueues[faceIdx]->m_cameraFar;
	dsInfo.m_directionalLight = (hasDirLight) ? &probe.m_renderQueues[faceIdx]->m_directionalLight : nullptr;
	dsInfo.m_pointLights = rqueue.m_pointLights;
	dsInfo.m_spotLights = rqueue.m_spotLights;
	dsInfo.m_commandBuffer = cmdb;
	dsInfo.m_gbufferRenderTargets[0] = m_ctx.m_gbufferColorRts[0];
	dsInfo.m_gbufferRenderTargets[1] = m_ctx.m_gbufferColorRts[1];
	dsInfo.m_gbufferRenderTargets[2] = m_ctx.m_gbufferColorRts[2];
	dsInfo.m_gbufferDepthRenderTarget = m_ctx.m_gbufferDepthRt;
	if(hasDirLight && dsInfo.m_directionalLight->hasShadow())
	{
		dsInfo.m_directionalLightShadowmapRenderTarget = m_ctx.m_shadowMapRt;
	}
	dsInfo.m_renderpassContext = &rgraphCtx;
	dsInfo.m_skybox = &rctx.m_renderQueue->m_skybox;

	m_lightShading.m_deferred.drawLights(dsInfo);
}

void ProbeReflections::runMipmappingOfLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(faceIdx < 6);
	ANKI_ASSERT(m_ctx.m_cacheEntryIdx < m_cacheEntries.getSize());

	ANKI_TRACE_SCOPED_EVENT(R_CUBE_REFL);

	TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, m_ctx.m_cacheEntryIdx));
	subresource.m_mipmapCount = m_lightShading.m_mipCount;

	TexturePtr texToBind;
	rgraphCtx.getRenderTargetState(m_ctx.m_lightShadingRt, subresource, texToBind);

	TextureViewInitInfo viewInit(texToBind, subresource);
	rgraphCtx.m_commandBuffer->generateMipmaps2d(getGrManager().newTextureView(viewInit));
}

void ProbeReflections::runIrradiance(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(R_CUBE_REFL);
	const U32 cacheEntryIdx = m_ctx.m_cacheEntryIdx;
	ANKI_ASSERT(cacheEntryIdx < m_cacheEntries.getSize());

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_irradiance.m_grProg);

	// Bind stuff
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);

	TextureSubresourceInfo subresource;
	subresource.m_faceCount = 6;
	subresource.m_firstLayer = cacheEntryIdx;
	rgraphCtx.bindTexture(0, 1, m_ctx.m_lightShadingRt, subresource);

	cmdb->bindStorageBuffer(0, 3, m_irradiance.m_diceValuesBuff, 0, m_irradiance.m_diceValuesBuff->getSize());

	// Draw
	cmdb->dispatchCompute(1, 1, 1);
}

void ProbeReflections::runIrradianceToRefl(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(R_CUBE_REFL);

	const U32 cacheEntryIdx = m_ctx.m_cacheEntryIdx;
	ANKI_ASSERT(cacheEntryIdx < m_cacheEntries.getSize());

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_irradianceToRefl.m_grProg);

	// Bind resources
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);

	rgraphCtx.bindColorTexture(0, 1, m_ctx.m_gbufferColorRts[0], 0);
	rgraphCtx.bindColorTexture(0, 1, m_ctx.m_gbufferColorRts[1], 1);
	rgraphCtx.bindColorTexture(0, 1, m_ctx.m_gbufferColorRts[2], 2);

	cmdb->bindStorageBuffer(0, 2, m_irradiance.m_diceValuesBuff, 0, m_irradiance.m_diceValuesBuff->getSize());

	TextureSubresourceInfo subresource;
	subresource.m_faceCount = 6;
	subresource.m_firstLayer = cacheEntryIdx;
	rgraphCtx.bindImage(0, 3, m_ctx.m_lightShadingRt, subresource);

	dispatchPPCompute(cmdb, 8, 8, m_lightShading.m_tileSize, m_lightShading.m_tileSize);
}

void ProbeReflections::populateRenderGraph(RenderingContext& rctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_CUBE_REFL);

#if ANKI_EXTRA_CHECKS
	m_ctx = {};
#endif
	RenderGraphDescription& rgraph = rctx.m_renderGraphDescr;

	// Prepare the probes and maybe get one to render this frame
	ReflectionProbeQueueElement* probeToUpdate;
	U32 probeToUpdateCacheEntryIdx;
	prepareProbes(rctx, probeToUpdate, probeToUpdateCacheEntryIdx);

	// Render a probe if needed
	if(!probeToUpdate)
	{
		// Just import and exit

		m_ctx.m_lightShadingRt = rgraph.importRenderTarget(m_lightShading.m_cubeArr, TextureUsageBit::kSampledFragment);
		return;
	}

	m_ctx.m_cacheEntryIdx = probeToUpdateCacheEntryIdx;
	m_ctx.m_probe = probeToUpdate;

	if(!m_cacheEntries[probeToUpdateCacheEntryIdx].m_lightShadingFbDescrs[0].isBacked())
	{
		initCacheEntry(probeToUpdateCacheEntryIdx);
	}

	// G-buffer pass
	{
		// RTs
		Array<RenderTargetHandle, kMaxColorRenderTargets> rts;
		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			m_ctx.m_gbufferColorRts[i] = rgraph.newRenderTarget(m_gbuffer.m_colorRtDescrs[i]);
			rts[i] = m_ctx.m_gbufferColorRts[i];
		}
		m_ctx.m_gbufferDepthRt = rgraph.newRenderTarget(m_gbuffer.m_depthRtDescr);

		// Compute task count
		m_ctx.m_gbufferRenderableCount = 0;
		for(U32 i = 0; i < 6; ++i)
		{
			m_ctx.m_gbufferRenderableCount += probeToUpdate->m_renderQueues[i]->m_renderables.getSize();
		}
		const U32 taskCount = computeNumberOfSecondLevelCommandBuffers(m_ctx.m_gbufferRenderableCount);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("CubeRefl gbuff");
		pass.setFramebufferInfo(m_gbuffer.m_fbDescr, rts, m_ctx.m_gbufferDepthRt);
		pass.setWork(taskCount, [this](RenderPassWorkContext& rgraphCtx) {
			runGBuffer(rgraphCtx);
		});

		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			pass.newTextureDependency(m_ctx.m_gbufferColorRts[i], TextureUsageBit::kFramebufferWrite);
		}

		TextureSubresourceInfo subresource(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(m_ctx.m_gbufferDepthRt, TextureUsageBit::kAllFramebuffer, subresource);
	}

	// Shadow pass. Optional
	if(probeToUpdate->m_renderQueues[0]->m_directionalLight.m_uuid
	   && probeToUpdate->m_renderQueues[0]->m_directionalLight.m_shadowCascadeCount > 0)
	{
		// Update light matrices
		for(U i = 0; i < 6; ++i)
		{
			ANKI_ASSERT(probeToUpdate->m_renderQueues[i]->m_directionalLight.m_uuid
						&& probeToUpdate->m_renderQueues[i]->m_directionalLight.m_shadowCascadeCount == 1);

			const F32 xScale = 1.0f / 6.0f;
			const F32 yScale = 1.0f;
			const F32 xOffset = F32(i) * (1.0f / 6.0f);
			const F32 yOffset = 0.0f;
			const Mat4 atlasMtx(xScale, 0.0f, 0.0f, xOffset, 0.0f, yScale, 0.0f, yOffset, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f);

			Mat4& lightMat = probeToUpdate->m_renderQueues[i]->m_directionalLight.m_textureMatrices[0];
			lightMat = atlasMtx * lightMat;
		}

		// Compute task count
		m_ctx.m_shadowRenderableCount = 0;
		for(U32 i = 0; i < 6; ++i)
		{
			m_ctx.m_shadowRenderableCount +=
				probeToUpdate->m_renderQueues[i]->m_directionalLight.m_shadowRenderQueues[0]->m_renderables.getSize();
		}
		const U32 taskCount = computeNumberOfSecondLevelCommandBuffers(m_ctx.m_shadowRenderableCount);

		// RT
		m_ctx.m_shadowMapRt = rgraph.newRenderTarget(m_shadowMapping.m_rtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("CubeRefl SM");
		pass.setFramebufferInfo(m_shadowMapping.m_fbDescr, {}, m_ctx.m_shadowMapRt);
		pass.setWork(taskCount, [this](RenderPassWorkContext& rgraphCtx) {
			runShadowMapping(rgraphCtx);
		});

		TextureSubresourceInfo subresource(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(m_ctx.m_shadowMapRt, TextureUsageBit::kAllFramebuffer, subresource);
	}
	else
	{
		m_ctx.m_shadowMapRt = {};
	}

	// Light shading passes
	{
		// RT
		m_ctx.m_lightShadingRt = rgraph.importRenderTarget(m_lightShading.m_cubeArr, TextureUsageBit::kSampledFragment);

		// Passes
		static constexpr Array<CString, 6> passNames = {"CubeRefl LightShad #0", "CubeRefl LightShad #1",
														"CubeRefl LightShad #2", "CubeRefl LightShad #3",
														"CubeRefl LightShad #4", "CubeRefl LightShad #5"};
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[faceIdx]);
			pass.setFramebufferInfo(m_cacheEntries[probeToUpdateCacheEntryIdx].m_lightShadingFbDescrs[faceIdx],
									{m_ctx.m_lightShadingRt});
			pass.setWork([this, faceIdx, &rctx](RenderPassWorkContext& rgraphCtx) {
				runLightShading(faceIdx, rctx, rgraphCtx);
			});

			TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, probeToUpdateCacheEntryIdx));
			pass.newTextureDependency(m_ctx.m_lightShadingRt, TextureUsageBit::kFramebufferWrite, subresource);

			for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
			{
				pass.newTextureDependency(m_ctx.m_gbufferColorRts[i], TextureUsageBit::kSampledFragment);
			}
			pass.newTextureDependency(m_ctx.m_gbufferDepthRt, TextureUsageBit::kSampledFragment,
									  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

			if(m_ctx.m_shadowMapRt.isValid())
			{
				pass.newTextureDependency(m_ctx.m_shadowMapRt, TextureUsageBit::kSampledFragment);
			}
		}
	}

	// Irradiance passes
	{
		m_ctx.m_irradianceDiceValuesBuffHandle =
			rgraph.importBuffer(m_irradiance.m_diceValuesBuff, BufferUsageBit::kNone);

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("CubeRefl Irradiance");

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			runIrradiance(rgraphCtx);
		});

		// Read a cube but only one layer and level
		TextureSubresourceInfo readSubresource;
		readSubresource.m_faceCount = 6;
		readSubresource.m_firstLayer = probeToUpdateCacheEntryIdx;
		pass.newTextureDependency(m_ctx.m_lightShadingRt, TextureUsageBit::kSampledCompute, readSubresource);

		pass.newBufferDependency(m_ctx.m_irradianceDiceValuesBuffHandle, BufferUsageBit::kStorageComputeWrite);
	}

	// Write irradiance back to refl
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("CubeRefl apply indirect");

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			runIrradianceToRefl(rgraphCtx);
		});

		for(U i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
		{
			pass.newTextureDependency(m_ctx.m_gbufferColorRts[i], TextureUsageBit::kSampledCompute);
		}

		TextureSubresourceInfo subresource;
		subresource.m_faceCount = 6;
		subresource.m_firstLayer = probeToUpdateCacheEntryIdx;
		pass.newTextureDependency(m_ctx.m_lightShadingRt,
								  TextureUsageBit::kImageComputeRead | TextureUsageBit::kImageComputeWrite,
								  subresource);

		pass.newBufferDependency(m_ctx.m_irradianceDiceValuesBuffHandle, BufferUsageBit::kStorageComputeRead);
	}

	// Mipmapping "passes"
	{
		static constexpr Array<CString, 6> passNames = {"CubeRefl Mip #0", "CubeRefl Mip #1", "CubeRefl Mip #2",
														"CubeRefl Mip #3", "CubeRefl Mip #4", "CubeRefl Mip #5"};
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[faceIdx]);
			pass.setWork([this, faceIdx](RenderPassWorkContext& rgraphCtx) {
				runMipmappingOfLightShading(faceIdx, rgraphCtx);
			});

			TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, probeToUpdateCacheEntryIdx));
			subresource.m_mipmapCount = m_lightShading.m_mipCount;

			pass.newTextureDependency(m_ctx.m_lightShadingRt, TextureUsageBit::kGenerateMipmaps, subresource);
		}
	}
}

void ProbeReflections::runShadowMapping(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_ctx.m_probe);
	ANKI_TRACE_SCOPED_EVENT(R_CUBE_REFL);

	I32 start, end;
	U32 startu, endu;
	splitThreadedProblem(rgraphCtx.m_currentSecondLevelCommandBufferIndex, rgraphCtx.m_secondLevelCommandBufferCount,
						 m_ctx.m_shadowRenderableCount, startu, endu);
	start = I32(startu);
	end = I32(endu);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->setPolygonOffset(1.0f, 1.0f);

	I32 drawcallCount = 0;
	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		ANKI_ASSERT(m_ctx.m_probe->m_renderQueues[faceIdx]);
		const RenderQueue& faceRenderQueue = *m_ctx.m_probe->m_renderQueues[faceIdx];
		ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_uuid != 0);
		ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_shadowCascadeCount == 1);
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
			args.m_minLod = args.m_maxLod = kMaxLodCount - 1;

			m_r->getSceneDrawer().drawRange(RenderingTechnique::kShadow, args,
											cascadeRenderQueue.m_renderables.getBegin() + localStart,
											cascadeRenderQueue.m_renderables.getBegin() + localEnd, cmdb);
		}
	}
}

} // end namespace anki
