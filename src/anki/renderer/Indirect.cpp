// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Indirect.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/Config.h>
#include <anki/core/Trace.h>
#include <anki/resource/MeshResource.h>

namespace anki
{

struct Indirect::LightPassVertexUniforms
{
	Mat4 m_mvp;
};

struct Indirect::LightPassPointLightUniforms
{
	Vec4 m_inputTexUvScaleAndOffset;
	Mat4 m_invViewProjMat;
	Vec4 m_camPosPad1;
	Vec4 m_posRadius;
	Vec4 m_diffuseColorPad1;
};

struct Indirect::LightPassSpotLightUniforms
{
	Vec4 m_inputTexUvScaleAndOffset;
	Mat4 m_invViewProjMat;
	Vec4 m_camPosPad1;
	Vec4 m_posRadius;
	Vec4 m_diffuseColorOuterCos;
	Vec4 m_lightDirInnerCos;
};

Indirect::Indirect(Renderer* r)
	: RendererObject(r)
{
}

Indirect::~Indirect()
{
	m_cacheEntries.destroy(getAllocator());
	m_probeUuidToCacheEntryIdx.destroy(getAllocator());
}

Error Indirect::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing image reflections");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize image reflections");
	}

	return err;
}

Error Indirect::initInternal(const ConfigSet& config)
{
	// Init cache entries
	m_cacheEntries.create(getAllocator(), config.getNumber("r.indirect.maxSimultaneousProbeCount"));

	ANKI_CHECK(initGBuffer(config));
	ANKI_CHECK(initLightShading(config));
	ANKI_CHECK(initIrradiance(config));

	// Load split sum integration LUT
	ANKI_CHECK(getResourceManager().loadResource("engine_data/SplitSumIntegration.ankitex", m_integrationLut));

	SamplerInitInfo sinit;
	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	sinit.m_mipmapFilter = SamplingFilter::BASE;
	sinit.m_minLod = 0.0;
	sinit.m_maxLod = 1.0;
	sinit.m_repeat = false;
	m_integrationLutSampler = getGrManager().newSampler(sinit);

	return Error::NONE;
}

Error Indirect::initGBuffer(const ConfigSet& config)
{
	m_gbuffer.m_tileSize = config.getNumber("r.indirect.reflectionResolution");

	// Create RT descriptions
	{
		RenderTargetDescription texinit = m_r->create2DRenderTargetDescription(m_gbuffer.m_tileSize * 6,
			m_gbuffer.m_tileSize,
			MS_COLOR_ATTACHMENT_PIXEL_FORMATS[0],
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			"GI GBuffer");

		// Create color RT descriptions
		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			texinit.m_format = MS_COLOR_ATTACHMENT_PIXEL_FORMATS[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(StringAuto(getAllocator()).sprintf("GI GBuff Col #%u", i).toCString());
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_usage |= TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
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
			m_gbuffer.m_fbDescr.m_colorAttachments[j].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		}

		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

		m_gbuffer.m_fbDescr.bake();
	}

	return Error::NONE;
}

Error Indirect::initLightShading(const ConfigSet& config)
{
	m_lightShading.m_tileSize = config.getNumber("r.indirect.reflectionResolution");
	m_lightShading.m_mipCount = computeMaxMipmapCount2d(m_lightShading.m_tileSize, m_lightShading.m_tileSize, 8);

	// Init cube arr
	{
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(m_lightShading.m_tileSize,
			m_lightShading.m_tileSize,
			LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE
				| TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::GENERATE_MIPMAPS,
			"GI refl");
		texinit.m_mipmapCount = m_lightShading.m_mipCount;
		texinit.m_type = TextureType::CUBE_ARRAY;
		texinit.m_layerCount = m_cacheEntries.getSize();
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;

		m_lightShading.m_cubeArr = m_r->createAndClearRenderTarget(texinit);
	}

	// Init progs
	{
		ANKI_CHECK(getResourceManager().loadResource("programs/DeferredShading.ankiprog", m_lightShading.m_lightProg));

		ShaderProgramResourceMutationInitList<1> mutators(m_lightShading.m_lightProg);
		mutators.add("LIGHT_TYPE", 0);

		ShaderProgramResourceConstantValueInitList<1> consts(m_lightShading.m_lightProg);
		consts.add("FB_SIZE", UVec2(m_lightShading.m_tileSize, m_lightShading.m_tileSize));

		const ShaderProgramResourceVariant* variant;
		m_lightShading.m_lightProg->getOrCreateVariant(mutators.get(), consts.get(), variant);
		m_lightShading.m_plightGrProg = variant->getProgram();

		mutators[0].m_value = 1;
		m_lightShading.m_lightProg->getOrCreateVariant(mutators.get(), consts.get(), variant);
		m_lightShading.m_slightGrProg = variant->getProgram();
	}

	// Init meshes
	ANKI_CHECK(getResourceManager().loadResource("engine_data/Plight.ankimesh", m_lightShading.m_plightMesh, false));
	ANKI_CHECK(getResourceManager().loadResource("engine_data/Slight.ankimesh", m_lightShading.m_slightMesh, false));

	return Error::NONE;
}

Error Indirect::initIrradiance(const ConfigSet& config)
{
	// Init atlas
	{
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(m_irradiance.m_tileSize,
			m_irradiance.m_tileSize,
			LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE
				| TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			"GI irr");

		texinit.m_layerCount = m_cacheEntries.getSize();
		texinit.m_type = TextureType::CUBE_ARRAY;
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;

		m_irradiance.m_cubeArr = m_r->createAndClearRenderTarget(texinit);
	}

	// Create prog
	{
		ANKI_CHECK(m_r->getResourceManager().loadResource("programs/Irradiance.ankiprog", m_irradiance.m_prog));

		const F32 envMapReadMip = computeMaxMipmapCount2d(
			m_lightShading.m_tileSize, m_lightShading.m_tileSize, m_irradiance.m_envMapReadSize);

		ShaderProgramResourceConstantValueInitList<2> consts(m_irradiance.m_prog);
		consts.add("ENV_TEX_TILE_SIZE", U32(m_irradiance.m_envMapReadSize));
		consts.add("ENV_TEX_MIP", envMapReadMip);

		const ShaderProgramResourceVariant* variant;
		m_irradiance.m_prog->getOrCreateVariant(consts.get(), variant);
		m_irradiance.m_grProg = variant->getProgram();
	}

	return Error::NONE;
}

void Indirect::initCacheEntry(U32 cacheEntryIdx)
{
	CacheEntry& cacheEntry = m_cacheEntries[cacheEntryIdx];

	for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		// Light pass FB
		{
			FramebufferDescription& fbDescr = cacheEntry.m_lightShadingFbDescrs[faceIdx];
			ANKI_ASSERT(!fbDescr.isBacked());
			fbDescr.m_colorAttachmentCount = 1;
			fbDescr.m_colorAttachments[0].m_surface.m_layer = cacheEntryIdx;
			fbDescr.m_colorAttachments[0].m_surface.m_face = faceIdx;
			fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
			fbDescr.bake();
		}

		// Irradiance FB
		{
			FramebufferDescription& fbDescr = cacheEntry.m_irradianceFbDescrs[faceIdx];
			ANKI_ASSERT(!fbDescr.isBacked());
			fbDescr.m_colorAttachmentCount = 1;
			fbDescr.m_colorAttachments[0].m_surface.m_layer = cacheEntryIdx;
			fbDescr.m_colorAttachments[0].m_surface.m_face = faceIdx;
			fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
			fbDescr.bake();
		}
	}
}

void Indirect::prepareProbes(
	RenderingContext& ctx, ReflectionProbeQueueElement*& probeToUpdate, U32& probeToUpdateCacheEntryIdx)
{
	probeToUpdate = nullptr;
	probeToUpdateCacheEntryIdx = MAX_U32;

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
	U newListOfProbeCount = 0;

	Bool foundProbeToRenderNextFrame = false;
	for(U32 probeIdx = 0; probeIdx < ctx.m_renderQueue->m_reflectionProbes.getSize(); ++probeIdx)
	{
		ReflectionProbeQueueElement& probe = ctx.m_renderQueue->m_reflectionProbes[probeIdx];

		// Find cache entry
		U32 cacheEntryIdx = MAX_U32;
		Bool cacheEntryFoundInCache;
		const Bool allocFailed = findBestCacheEntry(probe.m_uuid, cacheEntryIdx, cacheEntryFoundInCache);
		if(ANKI_UNLIKELY(allocFailed))
		{
			continue;
		}

		// Check if we _should_ and _can_ update the probe
		const Bool probeNeedsUpdate = m_cacheEntries[cacheEntryIdx].m_probeUuid != probe.m_uuid;
		if(ANKI_UNLIKELY(probeNeedsUpdate))
		{
			const Bool updateFailed = probeToUpdate != nullptr || probe.m_renderQueues[0] == nullptr;

			if(updateFailed && !foundProbeToRenderNextFrame)
			{
				// Probe will be updated next frame
				foundProbeToRenderNextFrame = true;
				probe.m_feedbackCallback(true, probe.m_userData);
			}

			if(updateFailed)
			{
				continue;
			}
		}

		// All good, can use this probe in this frame

		// Update the cache entry
		m_cacheEntries[cacheEntryIdx].m_probeUuid = probe.m_uuid;
		m_cacheEntries[cacheEntryIdx].m_lastUsedTimestamp = m_r->getGlobalTimestamp();

		// Update the probe
		probe.m_textureArrayIndex = cacheEntryIdx;

		// Push the probe to the new list
		newListOfProbes[newListOfProbeCount++] = probe;

		// Update cache map
		if(!cacheEntryFoundInCache)
		{
			m_probeUuidToCacheEntryIdx.emplace(getAllocator(), probe.m_uuid, cacheEntryIdx);
		}

		// Don't gather renderables next frame
		if(probe.m_renderQueues[0] != nullptr)
		{
			probe.m_feedbackCallback(false, probe.m_userData);
		}

		// Inform about the probe to update this frame
		if(probeNeedsUpdate)
		{
			ANKI_ASSERT(probe.m_renderQueues[0] != nullptr && probeToUpdate == nullptr);

			probeToUpdateCacheEntryIdx = cacheEntryIdx;
			probeToUpdate = &newListOfProbes[newListOfProbeCount - 1];
		}
	}

	// Replace the probe list in the queue
	if(newListOfProbeCount > 0)
	{
		ReflectionProbeQueueElement* firstProbe;
		PtrSize probeCount, storage;
		newListOfProbes.moveAndReset(firstProbe, probeCount, storage);
		ctx.m_renderQueue->m_reflectionProbes = WeakArray<ReflectionProbeQueueElement>(firstProbe, newListOfProbeCount);
	}
	else
	{
		ctx.m_renderQueue->m_reflectionProbes = WeakArray<ReflectionProbeQueueElement>();
		newListOfProbes.destroy(ctx.m_tempAllocator);
	}
}

void Indirect::runGBuffer(CommandBufferPtr& cmdb)
{
	ANKI_ASSERT(m_ctx.m_probe);
	ANKI_TRACE_SCOPED_EVENT(RENDER_IR);
	const ReflectionProbeQueueElement& probe = *m_ctx.m_probe;

	// For each face
	for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		const U32 viewportX = faceIdx * m_gbuffer.m_tileSize;
		cmdb->setViewport(viewportX, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);
		cmdb->setScissor(viewportX, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);

		/// Draw
		ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
		const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

		m_r->getSceneDrawer().drawRange(Pass::GB_FS,
			rqueue.m_viewMatrix,
			rqueue.m_viewProjectionMatrix,
			cmdb,
			rqueue.m_renderables.getBegin(),
			rqueue.m_renderables.getEnd());
	}

	// Restore state
	cmdb->setScissor(0, 0, MAX_U32, MAX_U32);
}

void Indirect::bindVertexIndexBuffers(MeshResourcePtr& mesh, CommandBufferPtr& cmdb, U32& indexCount)
{
	// Attrib
	U32 bufferBinding;
	Format fmt;
	PtrSize relativeOffset;
	mesh->getVertexAttributeInfo(VertexAttributeLocation::POSITION, bufferBinding, fmt, relativeOffset);

	cmdb->setVertexAttribute(0, 0, fmt, relativeOffset);

	// Vert buff
	BufferPtr buff;
	PtrSize offset, stride;
	mesh->getVertexBufferInfo(bufferBinding, buff, offset, stride);

	cmdb->bindVertexBuffer(0, buff, offset, stride);

	// Idx buff
	IndexType idxType;
	mesh->getIndexBufferInfo(buff, offset, indexCount, idxType);

	cmdb->bindIndexBuffer(buff, offset, idxType);
}

void Indirect::runLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(faceIdx <= 6);
	ANKI_TRACE_SCOPED_EVENT(RENDER_IR);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	ANKI_ASSERT(m_ctx.m_probe);
	const ReflectionProbeQueueElement& probe = *m_ctx.m_probe;

	// Set common state for all lights
	// NOTE: Use nearest sampler because we don't want the result to sample the near tiles
	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
	{
		rgraphCtx.bindColorTextureAndSampler(0, i, m_ctx.m_gbufferColorRts[i], m_r->getNearestSampler());
	}
	rgraphCtx.bindTextureAndSampler(0,
		GBUFFER_COLOR_ATTACHMENT_COUNT,
		m_ctx.m_gbufferDepthRt,
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
		m_r->getNearestSampler());
	cmdb->setViewport(0, 0, m_lightShading.m_tileSize, m_lightShading.m_tileSize);
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->setCullMode(FaceSelectionBit::FRONT);

	// Render lights
	{
		ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
		const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

		const Mat4& vpMat = rqueue.m_viewProjectionMatrix;
		const Mat4 invViewProjMat = rqueue.m_viewProjectionMatrix.getInverse();

		// Do point lights
		U32 indexCount;
		bindVertexIndexBuffers(m_lightShading.m_plightMesh, cmdb, indexCount);
		cmdb->bindShaderProgram(m_lightShading.m_plightGrProg);

		const PointLightQueueElement* plightEl = rqueue.m_pointLights.getBegin();
		while(plightEl != rqueue.m_pointLights.getEnd())
		{
			// Update uniforms
			LightPassVertexUniforms* vert =
				allocateAndBindUniforms<LightPassVertexUniforms*>(sizeof(LightPassVertexUniforms), cmdb, 0, 0);

			Mat4 modelM(plightEl->m_worldPosition.xyz1(), Mat3::getIdentity(), plightEl->m_radius);

			vert->m_mvp = vpMat * modelM;

			LightPassPointLightUniforms* light =
				allocateAndBindUniforms<LightPassPointLightUniforms*>(sizeof(LightPassPointLightUniforms), cmdb, 0, 1);

			light->m_inputTexUvScaleAndOffset = Vec4(1.0f / 6.0f, 1.0f, faceIdx * (1.0f / 6.0f), 0.0f);
			light->m_invViewProjMat = invViewProjMat;
			light->m_camPosPad1 = rqueue.m_cameraTransform.getTranslationPart();
			light->m_posRadius =
				Vec4(plightEl->m_worldPosition.xyz(), 1.0f / (plightEl->m_radius * plightEl->m_radius));
			light->m_diffuseColorPad1 = plightEl->m_diffuseColor.xyz0();

			// Draw
			cmdb->drawElements(PrimitiveTopology::TRIANGLES, indexCount);

			++plightEl;
		}

		// Do spot lights
		bindVertexIndexBuffers(m_lightShading.m_slightMesh, cmdb, indexCount);
		cmdb->bindShaderProgram(m_lightShading.m_slightGrProg);

		const SpotLightQueueElement* splightEl = rqueue.m_spotLights.getBegin();
		while(splightEl != rqueue.m_spotLights.getEnd())
		{
			// Compute the model matrix
			//
			Mat4 modelM(splightEl->m_worldTransform.getTranslationPart().xyz1(),
				splightEl->m_worldTransform.getRotationPart(),
				1.0f);

			// Calc the scale of the cone
			Mat4 scaleM(Mat4::getIdentity());
			scaleM(0, 0) = tan(splightEl->m_outerAngle / 2.0f) * splightEl->m_distance;
			scaleM(1, 1) = scaleM(0, 0);
			scaleM(2, 2) = splightEl->m_distance;

			modelM = modelM * scaleM;

			// Update vertex uniforms
			LightPassVertexUniforms* vert =
				allocateAndBindUniforms<LightPassVertexUniforms*>(sizeof(LightPassVertexUniforms), cmdb, 0, 0);
			vert->m_mvp = vpMat * modelM;

			// Update fragment uniforms
			LightPassSpotLightUniforms* light =
				allocateAndBindUniforms<LightPassSpotLightUniforms*>(sizeof(LightPassSpotLightUniforms), cmdb, 0, 1);

			light->m_inputTexUvScaleAndOffset = Vec4(1.0f / 6.0f, 1.0f, faceIdx * (1.0f / 6.0f), 0.0f);
			light->m_invViewProjMat = invViewProjMat;
			light->m_camPosPad1 = rqueue.m_cameraTransform.getTranslationPart();

			light->m_posRadius = Vec4(splightEl->m_worldTransform.getTranslationPart().xyz(),
				1.0f / (splightEl->m_distance * splightEl->m_distance));

			light->m_diffuseColorOuterCos = Vec4(splightEl->m_diffuseColor, cos(splightEl->m_outerAngle / 2.0f));

			Vec3 lightDir = -splightEl->m_worldTransform.getZAxis().xyz();
			light->m_lightDirInnerCos = Vec4(lightDir, cos(splightEl->m_innerAngle / 2.0f));

			// Draw
			cmdb->drawElements(PrimitiveTopology::TRIANGLES, indexCount);

			++splightEl;
		}
	}

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setCullMode(FaceSelectionBit::BACK);
}

void Indirect::runMipmappingOfLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(faceIdx < 6);
	ANKI_ASSERT(m_ctx.m_cacheEntryIdx < m_cacheEntries.getSize());

	ANKI_TRACE_SCOPED_EVENT(RENDER_IR);

	TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, m_ctx.m_cacheEntryIdx));
	subresource.m_mipmapCount = m_lightShading.m_mipCount;

	TexturePtr texToBind;
	TextureUsageBit usage;
	rgraphCtx.getRenderTargetState(m_ctx.m_lightShadingRt, subresource, texToBind, usage);

	TextureViewInitInfo viewInit(texToBind, subresource);
	rgraphCtx.m_commandBuffer->generateMipmaps2d(getGrManager().newTextureView(viewInit));
}

void Indirect::runIrradiance(U32 faceIdx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(faceIdx < 6);
	ANKI_TRACE_SCOPED_EVENT(RENDER_IR);
	const U32 cacheEntryIdx = m_ctx.m_cacheEntryIdx;
	ANKI_ASSERT(cacheEntryIdx < m_cacheEntries.getSize());

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	// Set state
	cmdb->bindShaderProgram(m_irradiance.m_grProg);
	rgraphCtx.bindColorTextureAndSampler(0, 0, m_ctx.m_lightShadingRt, m_r->getLinearSampler());
	cmdb->setViewport(0, 0, m_irradiance.m_tileSize, m_irradiance.m_tileSize);

	// Set uniforms
	UVec4* faceIdxArrayIdx = allocateAndBindUniforms<UVec4*>(sizeof(UVec4), cmdb, 0, 0);
	faceIdxArrayIdx->x() = faceIdx;
	faceIdxArrayIdx->y() = cacheEntryIdx;

	// Draw
	drawQuad(cmdb);
}

void Indirect::populateRenderGraph(RenderingContext& rctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_IR);

#if ANKI_EXTRA_CHECKS
	m_ctx = {};
#endif
	RenderGraphDescription& rgraph = rctx.m_renderGraphDescr;

	// Prepare the probes and maybe get one to render this frame
	ReflectionProbeQueueElement* probeToUpdate;
	U32 probeToUpdateCacheEntryIdx;
	prepareProbes(rctx, probeToUpdate, probeToUpdateCacheEntryIdx);

	// Render a probe if needed
	if(probeToUpdate)
	{
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

			// Pass
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI gbuff");
			pass.setFramebufferInfo(m_gbuffer.m_fbDescr, rts, m_ctx.m_gbufferDepthRt);
			pass.setWork(runGBufferCallback, this, 0);

			for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
			{
				pass.newConsumer({m_ctx.m_gbufferColorRts[i], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
				pass.newProducer({m_ctx.m_gbufferColorRts[i], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
			}

			TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
			pass.newConsumer({m_ctx.m_gbufferDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
			pass.newProducer({m_ctx.m_gbufferDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
		}

		// Light shading passes
		{
			Array<RenderPassWorkCallback, 6> callbacks = {{runLightShadingCallback<0>,
				runLightShadingCallback<1>,
				runLightShadingCallback<2>,
				runLightShadingCallback<3>,
				runLightShadingCallback<4>,
				runLightShadingCallback<5>}};

			// RT
			m_ctx.m_lightShadingRt =
				rgraph.importRenderTarget("GI light", m_lightShading.m_cubeArr, TextureUsageBit::SAMPLED_FRAGMENT);

			// Passes
			static const Array<CString, 6> passNames = {{"GI LightShad #0",
				"GI LightShad #1",
				"GI LightShad #2",
				"GI LightShad #3",
				"GI LightShad #4",
				"GI LightShad #5"}};
			for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
			{
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[faceIdx]);
				pass.setFramebufferInfo(m_cacheEntries[probeToUpdateCacheEntryIdx].m_lightShadingFbDescrs[faceIdx],
					{{m_ctx.m_lightShadingRt}},
					{});
				pass.setWork(callbacks[faceIdx], this, 0);

				TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, probeToUpdateCacheEntryIdx));
				pass.newConsumer({m_ctx.m_lightShadingRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource});
				pass.newProducer({m_ctx.m_lightShadingRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource});

				for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
				{
					pass.newConsumer({m_ctx.m_gbufferColorRts[i], TextureUsageBit::SAMPLED_FRAGMENT});
				}
				pass.newConsumer({m_ctx.m_gbufferDepthRt,
					TextureUsageBit::SAMPLED_FRAGMENT,
					TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});
			}
		}

		// Mipmapping "passes"
		{
			static const Array<RenderPassWorkCallback, 6> callbacks = {{runMipmappingOfLightShadingCallback<0>,
				runMipmappingOfLightShadingCallback<1>,
				runMipmappingOfLightShadingCallback<2>,
				runMipmappingOfLightShadingCallback<3>,
				runMipmappingOfLightShadingCallback<4>,
				runMipmappingOfLightShadingCallback<5>}};

			static const Array<CString, 6> passNames = {
				{"GI Mip #0", "GI Mip #1", "GI Mip #2", "GI Mip #3", "GI Mip #4", "GI Mip #5"}};
			for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
			{
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[faceIdx]);
				pass.setWork(callbacks[faceIdx], this, 0);

				TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, probeToUpdateCacheEntryIdx));
				subresource.m_mipmapCount = m_lightShading.m_mipCount;

				pass.newConsumer({m_ctx.m_lightShadingRt, TextureUsageBit::GENERATE_MIPMAPS, subresource});
				pass.newProducer({m_ctx.m_lightShadingRt, TextureUsageBit::GENERATE_MIPMAPS, subresource});
			}
		}

		// Irradiance passes
		{
			static const Array<RenderPassWorkCallback, 6> callbacks = {{runIrradianceCallback<0>,
				runIrradianceCallback<1>,
				runIrradianceCallback<2>,
				runIrradianceCallback<3>,
				runIrradianceCallback<4>,
				runIrradianceCallback<5>}};

			// Rt
			m_ctx.m_irradianceRt =
				rgraph.importRenderTarget("GI irradiance", m_irradiance.m_cubeArr, TextureUsageBit::SAMPLED_FRAGMENT);

			static const Array<CString, 6> passNames = {
				{"GI Irr/ce #0", "GI Irr/ce #1", "GI Irr/ce #2", "GI Irr/ce #3", "GI Irr/ce #4", "GI Irr/ce #5"}};
			for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
			{
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[faceIdx]);

				pass.setFramebufferInfo(m_cacheEntries[probeToUpdateCacheEntryIdx].m_irradianceFbDescrs[faceIdx],
					{{m_ctx.m_irradianceRt}},
					{});

				pass.setWork(callbacks[faceIdx], this, 0);

				pass.newConsumer({m_ctx.m_lightShadingRt, TextureUsageBit::SAMPLED_FRAGMENT});

				TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, probeToUpdateCacheEntryIdx));
				pass.newConsumer({m_ctx.m_irradianceRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource});
				pass.newProducer({m_ctx.m_irradianceRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource});
			}
		}
	}
	else
	{
		// Just import

		m_ctx.m_lightShadingRt =
			rgraph.importRenderTarget("GI light", m_lightShading.m_cubeArr, TextureUsageBit::SAMPLED_FRAGMENT);
		m_ctx.m_irradianceRt =
			rgraph.importRenderTarget("GI irradiance", m_irradiance.m_cubeArr, TextureUsageBit::SAMPLED_FRAGMENT);
	}
}

Bool Indirect::findBestCacheEntry(U64 probeUuid, U32& cacheEntryIdxAllocated, Bool& cacheEntryFound)
{
	ANKI_ASSERT(probeUuid > 0);

	// First, try to see if the probe is in the cache
	auto it = m_probeUuidToCacheEntryIdx.find(probeUuid);
	if(it != m_probeUuidToCacheEntryIdx.getEnd())
	{
		const U32 cacheEntryIdx = *it;
		if(m_cacheEntries[cacheEntryIdx].m_probeUuid == probeUuid)
		{
			// Found it
			cacheEntryIdxAllocated = cacheEntryIdx;
			cacheEntryFound = true;
			return false;
		}
		else
		{
			// Cache entry is wrong, remove it
			m_probeUuidToCacheEntryIdx.erase(getAllocator(), it);
		}
	}
	cacheEntryFound = false;

	// 2nd and 3rd choice, find an empty entry or some entry to re-use
	U32 emptyCacheEntryIdx = MAX_U32;
	U32 cacheEntryIdxToKick = MAX_U32;
	Timestamp cacheEntryIdxToKickMinTimestamp = MAX_TIMESTAMP;
	for(U32 cacheEntryIdx = 0; cacheEntryIdx < m_cacheEntries.getSize(); ++cacheEntryIdx)
	{
		if(m_cacheEntries[cacheEntryIdx].m_probeUuid == 0)
		{
			// Found an empty
			emptyCacheEntryIdx = cacheEntryIdx;
			break;
		}
		else if(m_cacheEntries[cacheEntryIdx].m_lastUsedTimestamp != m_r->getGlobalTimestamp()
				&& m_cacheEntries[cacheEntryIdx].m_lastUsedTimestamp < cacheEntryIdxToKickMinTimestamp)
		{
			// Found some with low timestamp
			cacheEntryIdxToKick = cacheEntryIdx;
			cacheEntryIdxToKickMinTimestamp = m_cacheEntries[cacheEntryIdx].m_lastUsedTimestamp;
		}
	}

	Bool failed = false;
	if(emptyCacheEntryIdx != MAX_U32)
	{
		cacheEntryIdxAllocated = emptyCacheEntryIdx;
	}
	else if(cacheEntryIdxToKick != MAX_U32)
	{
		cacheEntryIdxAllocated = cacheEntryIdxToKick;
	}
	else
	{
		// We have a problem
		failed = true;
		ANKI_R_LOGW("There is not enough space in the indirect lighting atlas for more probes. "
					"Increase the r.indirect.maxSimultaneousProbeCount or decrease the scene's probes");
	}

	return failed;
}

} // end namespace anki
