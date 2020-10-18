// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/ProbeReflections.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/ConfigSet.h>
#include <anki/util/Tracer.h>
#include <anki/resource/MeshResource.h>
#include <anki/shaders/include/TraditionalDeferredShadingTypes.h>

namespace anki
{

ProbeReflections::ProbeReflections(Renderer* r)
	: RendererObject(r)
	, m_lightShading(r)
{
}

ProbeReflections::~ProbeReflections()
{
	m_cacheEntries.destroy(getAllocator());
	m_probeUuidToCacheEntryIdx.destroy(getAllocator());
}

Error ProbeReflections::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing image reflections");

	const Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize image reflections");
	}

	return err;
}

Error ProbeReflections::initInternal(const ConfigSet& config)
{
	// Init cache entries
	m_cacheEntries.create(getAllocator(), config.getNumberU32("r_probeRefectionlMaxSimultaneousProbeCount"));

	ANKI_CHECK(initGBuffer(config));
	ANKI_CHECK(initLightShading(config));
	ANKI_CHECK(initIrradiance(config));
	ANKI_CHECK(initIrradianceToRefl(config));
	ANKI_CHECK(initShadowMapping(config));

	// Load split sum integration LUT
	ANKI_CHECK(getResourceManager().loadResource("engine_data/SplitSumIntegration.ankitex", m_integrationLut));

	SamplerInitInfo sinit;
	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	sinit.m_mipmapFilter = SamplingFilter::BASE;
	sinit.m_minLod = 0.0;
	sinit.m_maxLod = 1.0;
	sinit.m_addressing = SamplingAddressing::CLAMP;
	m_integrationLutSampler = getGrManager().newSampler(sinit);

	return Error::NONE;
}

Error ProbeReflections::initGBuffer(const ConfigSet& config)
{
	m_gbuffer.m_tileSize = config.getNumberU32("r_probeReflectionResolution");

	// Create RT descriptions
	{
		RenderTargetDescription texinit =
			m_r->create2DRenderTargetDescription(m_gbuffer.m_tileSize * 6, m_gbuffer.m_tileSize,
												 GBUFFER_COLOR_ATTACHMENT_PIXEL_FORMATS[0], "CubeRefl GBuffer");

		// Create color RT descriptions
		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			texinit.m_format = GBUFFER_COLOR_ATTACHMENT_PIXEL_FORMATS[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(
				StringAuto(getAllocator()).sprintf("CubeRefl GBuff Col #%u", i).toCString());
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_format = GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT;
		texinit.setName("CubeRefl GBuff Depth");
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

Error ProbeReflections::initLightShading(const ConfigSet& config)
{
	m_lightShading.m_tileSize = config.getNumberU32("r_probeReflectionResolution");
	m_lightShading.m_mipCount = computeMaxMipmapCount2d(m_lightShading.m_tileSize, m_lightShading.m_tileSize, 8);

	// Init cube arr
	{
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(
			m_lightShading.m_tileSize, m_lightShading.m_tileSize, LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE | TextureUsageBit::IMAGE_COMPUTE_READ
				| TextureUsageBit::IMAGE_COMPUTE_WRITE | TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT
				| TextureUsageBit::GENERATE_MIPMAPS,
			"CubeRefl refl");
		texinit.m_mipmapCount = U8(m_lightShading.m_mipCount);
		texinit.m_type = TextureType::CUBE_ARRAY;
		texinit.m_layerCount = m_cacheEntries.getSize();
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;

		m_lightShading.m_cubeArr = m_r->createAndClearRenderTarget(texinit);
	}

	// Init deferred
	ANKI_CHECK(m_lightShading.m_deferred.init());

	return Error::NONE;
}

Error ProbeReflections::initIrradiance(const ConfigSet& config)
{
	m_irradiance.m_workgroupSize = config.getNumberU32("r_probeReflectionIrradianceResolution");

	// Create prog
	{
		ANKI_CHECK(m_r->getResourceManager().loadResource("shaders/IrradianceDice.ankiprog", m_irradiance.m_prog));

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
		init.m_usage = BufferUsageBit::ALL_STORAGE;
		init.m_size = 6 * sizeof(Vec4);
		m_irradiance.m_diceValuesBuff = getGrManager().newBuffer(init);
	}

	return Error::NONE;
}

Error ProbeReflections::initIrradianceToRefl(const ConfigSet& cfg)
{
	// Create program
	ANKI_CHECK(m_r->getResourceManager().loadResource("shaders/ApplyIrradianceToReflection.ankiprog",
													  m_irradianceToRefl.m_prog));

	const ShaderProgramResourceVariant* variant;
	m_irradianceToRefl.m_prog->getOrCreateVariant(ShaderProgramResourceVariantInitInfo(m_irradianceToRefl.m_prog),
												  variant);
	m_irradianceToRefl.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error ProbeReflections::initShadowMapping(const ConfigSet& cfg)
{
	const U32 resolution = cfg.getNumberU32("r_probeReflectionShadowMapResolution");
	ANKI_ASSERT(resolution > 8);

	// RT descr
	m_shadowMapping.m_rtDescr =
		m_r->create2DRenderTargetDescription(resolution * 6, resolution, Format::D32_SFLOAT, "CubeRefl SM");
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
		fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
		fbDescr.bake();
	}
}

void ProbeReflections::prepareProbes(RenderingContext& ctx, ReflectionProbeQueueElement*& probeToUpdateThisFrame,
									 U32& probeToUpdateThisFrameCacheEntryIdx)
{
	probeToUpdateThisFrame = nullptr;
	probeToUpdateThisFrameCacheEntryIdx = MAX_U32;

	if(ANKI_UNLIKELY(ctx.m_renderQueue->m_reflectionProbes.getSize() == 0))
	{
		return;
	}

	// Iterate the probes and:
	// - Find a probe to update this frame
	// - Find a probe to update next frame
	// - Find the cache entries for each probe
	DynamicArray<ReflectionProbeQueueElement> newListOfProbes;
	newListOfProbes.create(ctx.m_tempAllocator, ctx.m_renderQueue->m_reflectionProbes.getSize());
	U32 newListOfProbeCount = 0;
	Bool foundProbeToUpdateNextFrame = false;
	for(U32 probeIdx = 0; probeIdx < ctx.m_renderQueue->m_reflectionProbes.getSize(); ++probeIdx)
	{
		ReflectionProbeQueueElement& probe = ctx.m_renderQueue->m_reflectionProbes[probeIdx];

		// Find cache entry
		const U32 cacheEntryIdx = findBestCacheEntry(probe.m_uuid, m_r->getGlobalTimestamp(), m_cacheEntries,
													 m_probeUuidToCacheEntryIdx, getAllocator());
		if(ANKI_UNLIKELY(cacheEntryIdx == MAX_U32))
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
			m_probeUuidToCacheEntryIdx.emplace(getAllocator(), probe.m_uuid, cacheEntryIdx);
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
		newListOfProbes.destroy(ctx.m_tempAllocator);
	}
}

void ProbeReflections::runGBuffer(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_ctx.m_probe);
	ANKI_TRACE_SCOPED_EVENT(R_CUBE_REFL);
	const ReflectionProbeQueueElement& probe = *m_ctx.m_probe;
	const CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

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
			m_r->getSceneDrawer().drawRange(Pass::GB, rqueue.m_viewMatrix, rqueue.m_viewProjectionMatrix,
											Mat4::getIdentity(), // Don't care about prev mats
											cmdb, m_r->getSamplers().m_trilinearRepeat,
											rqueue.m_renderables.getBegin() + localStart,
											rqueue.m_renderables.getBegin() + localEnd, MAX_LOD_COUNT - 1);
		}
	}

	// Restore state
	cmdb->setScissor(0, 0, MAX_U32, MAX_U32);
}

void ProbeReflections::runLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(faceIdx <= 6);
	ANKI_TRACE_SCOPED_EVENT(R_CUBE_REFL);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	ANKI_ASSERT(m_ctx.m_probe);
	const ReflectionProbeQueueElement& probe = *m_ctx.m_probe;
	const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

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
		ANKI_ASSERT(m_ctx.m_shadowMapRt.isValid());

		cmdb->bindSampler(0, 7, m_shadowMapping.m_shadowSampler);

		rgraphCtx.bindTexture(0, 8, m_ctx.m_shadowMapRt, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	}

	TraditionalDeferredLightShadingDrawInfo dsInfo;
	dsInfo.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
	dsInfo.m_invViewProjectionMatrix = rqueue.m_viewProjectionMatrix.getInverse();
	dsInfo.m_cameraPosWSpace = rqueue.m_cameraTransform.getTranslationPart();
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
	TextureUsageBit usage;
	rgraphCtx.getRenderTargetState(m_ctx.m_lightShadingRt, subresource, texToBind, usage);

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

	allocateAndBindStorage<void*>(sizeof(Vec4) * 6 * m_irradiance.m_workgroupSize * m_irradiance.m_workgroupSize, cmdb,
								  0, 3);

	cmdb->bindStorageBuffer(0, 4, m_irradiance.m_diceValuesBuff, 0, m_irradiance.m_diceValuesBuff->getSize());

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

		m_ctx.m_lightShadingRt = rgraph.importRenderTarget(m_lightShading.m_cubeArr, TextureUsageBit::SAMPLED_FRAGMENT);
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
		Array<RenderTargetHandle, MAX_COLOR_ATTACHMENTS> rts;
		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
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
		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<ProbeReflections*>(rgraphCtx.m_userData)->runGBuffer(rgraphCtx);
			},
			this, taskCount);

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			pass.newDependency({m_ctx.m_gbufferColorRts[i], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		}

		TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
		pass.newDependency({m_ctx.m_gbufferDepthRt, TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT, subresource});
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
		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<ProbeReflections*>(rgraphCtx.m_userData)->runShadowMapping(rgraphCtx);
			},
			this, taskCount);

		TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
		pass.newDependency({m_ctx.m_shadowMapRt, TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT, subresource});
	}
	else
	{
		m_ctx.m_shadowMapRt = {};
	}

	// Light shading passes
	{
		Array<RenderPassWorkCallback, 6> callbacks = {runLightShadingCallback<0>, runLightShadingCallback<1>,
													  runLightShadingCallback<2>, runLightShadingCallback<3>,
													  runLightShadingCallback<4>, runLightShadingCallback<5>};

		// RT
		m_ctx.m_lightShadingRt = rgraph.importRenderTarget(m_lightShading.m_cubeArr, TextureUsageBit::SAMPLED_FRAGMENT);

		// Passes
		static const Array<CString, 6> passNames = {"CubeRefl LightShad #0", "CubeRefl LightShad #1",
													"CubeRefl LightShad #2", "CubeRefl LightShad #3",
													"CubeRefl LightShad #4", "CubeRefl LightShad #5"};
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[faceIdx]);
			pass.setFramebufferInfo(m_cacheEntries[probeToUpdateCacheEntryIdx].m_lightShadingFbDescrs[faceIdx],
									{{m_ctx.m_lightShadingRt}}, {});
			pass.setWork(callbacks[faceIdx], this, 0);

			TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, probeToUpdateCacheEntryIdx));
			pass.newDependency({m_ctx.m_lightShadingRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource});

			for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
			{
				pass.newDependency({m_ctx.m_gbufferColorRts[i], TextureUsageBit::SAMPLED_FRAGMENT});
			}
			pass.newDependency({m_ctx.m_gbufferDepthRt, TextureUsageBit::SAMPLED_FRAGMENT,
								TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});

			if(m_ctx.m_shadowMapRt.isValid())
			{
				pass.newDependency({m_ctx.m_shadowMapRt, TextureUsageBit::SAMPLED_FRAGMENT});
			}
		}
	}

	// Irradiance passes
	{
		m_ctx.m_irradianceDiceValuesBuffHandle =
			rgraph.importBuffer(m_irradiance.m_diceValuesBuff, BufferUsageBit::NONE);

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("CubeRefl Irradiance");

		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<ProbeReflections*>(rgraphCtx.m_userData)->runIrradiance(rgraphCtx);
			},
			this, 0);

		// Read a cube but only one layer and level
		TextureSubresourceInfo readSubresource;
		readSubresource.m_faceCount = 6;
		readSubresource.m_firstLayer = probeToUpdateCacheEntryIdx;
		pass.newDependency({m_ctx.m_lightShadingRt, TextureUsageBit::SAMPLED_COMPUTE, readSubresource});

		pass.newDependency({m_ctx.m_irradianceDiceValuesBuffHandle, BufferUsageBit::STORAGE_COMPUTE_WRITE});
	}

	// Write irradiance back to refl
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("CubeRefl apply indirect");

		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<ProbeReflections*>(rgraphCtx.m_userData)->runIrradianceToRefl(rgraphCtx);
			},
			this, 0);

		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT - 1; ++i)
		{
			pass.newDependency({m_ctx.m_gbufferColorRts[i], TextureUsageBit::SAMPLED_COMPUTE});
		}

		TextureSubresourceInfo subresource;
		subresource.m_faceCount = 6;
		subresource.m_firstLayer = probeToUpdateCacheEntryIdx;
		pass.newDependency({m_ctx.m_lightShadingRt,
							TextureUsageBit::IMAGE_COMPUTE_READ | TextureUsageBit::IMAGE_COMPUTE_WRITE, subresource});

		pass.newDependency({m_ctx.m_irradianceDiceValuesBuffHandle, BufferUsageBit::STORAGE_COMPUTE_READ});
	}

	// Mipmapping "passes"
	{
		static const Array<RenderPassWorkCallback, 6> callbacks = {
			{runMipmappingOfLightShadingCallback<0>, runMipmappingOfLightShadingCallback<1>,
			 runMipmappingOfLightShadingCallback<2>, runMipmappingOfLightShadingCallback<3>,
			 runMipmappingOfLightShadingCallback<4>, runMipmappingOfLightShadingCallback<5>}};

		static const Array<CString, 6> passNames = {"CubeRefl Mip #0", "CubeRefl Mip #1", "CubeRefl Mip #2",
													"CubeRefl Mip #3", "CubeRefl Mip #4", "CubeRefl Mip #5"};
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[faceIdx]);
			pass.setWork(callbacks[faceIdx], this, 0);

			TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, probeToUpdateCacheEntryIdx));
			subresource.m_mipmapCount = m_lightShading.m_mipCount;

			pass.newDependency({m_ctx.m_lightShadingRt, TextureUsageBit::GENERATE_MIPMAPS, subresource});
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

	const CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
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
			m_r->getSceneDrawer().drawRange(Pass::SM, cascadeRenderQueue.m_viewMatrix,
											cascadeRenderQueue.m_viewProjectionMatrix,
											Mat4::getIdentity(), // Don't care about prev matrices here
											cmdb, m_r->getSamplers().m_trilinearRepeatAniso,
											cascadeRenderQueue.m_renderables.getBegin() + localStart,
											cascadeRenderQueue.m_renderables.getBegin() + localEnd, MAX_LOD_COUNT - 1);
		}
	}
}

} // end namespace anki
