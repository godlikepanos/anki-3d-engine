// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/resource/MeshLoader.h>

namespace anki
{

struct Indirect::LightPassVertexUniforms
{
	Mat4 m_mvp;
};

struct Indirect::LightPassPointLightUniforms
{
	Vec4 m_inputTexUvScaleAndOffset;
	Vec4 m_projectionParams;
	Vec4 m_posRadius;
	Vec4 m_diffuseColorPad1;
	Vec4 m_specularColorPad1;
};

struct Indirect::LightPassSpotLightUniforms
{
	Vec4 m_inputTexUvScaleAndOffset;
	Vec4 m_projectionParams;
	Vec4 m_posRadius;
	Vec4 m_diffuseColorOuterCos;
	Vec4 m_specularColorInnerCos;
	Vec4 m_lightDirPad1;
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
	m_integrationLutSampler = getGrManager().newInstance<Sampler>(sinit);

	return Error::NONE;
}

Error Indirect::loadMesh(CString fname, BufferPtr& vert, BufferPtr& idx, U32& idxCount)
{
	MeshLoader loader(&getResourceManager());
	ANKI_CHECK(loader.load(fname));

	PtrSize vertBuffSize = loader.getHeader().m_totalVerticesCount * sizeof(Vec3);
	vert = getGrManager().newInstance<Buffer>(
		vertBuffSize, BufferUsageBit::VERTEX | BufferUsageBit::BUFFER_UPLOAD_DESTINATION, BufferMapAccessBit::NONE);

	idx = getGrManager().newInstance<Buffer>(loader.getIndexDataSize(),
		BufferUsageBit::INDEX | BufferUsageBit::BUFFER_UPLOAD_DESTINATION,
		BufferMapAccessBit::NONE);

	// Upload data
	CommandBufferInitInfo init;
	init.m_flags = CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(init);

	TransferGpuAllocatorHandle handle;
	ANKI_CHECK(m_r->getResourceManager().getTransferGpuAllocator().allocate(vertBuffSize, handle));

	Vec3* verts = static_cast<Vec3*>(handle.getMappedMemory());

	const U8* ptr = loader.getVertexData();
	for(U i = 0; i < loader.getHeader().m_totalVerticesCount; ++i)
	{
		*verts = *reinterpret_cast<const Vec3*>(ptr);
		++verts;
		ptr += loader.getVertexSize();
	}

	cmdb->copyBufferToBuffer(handle.getBuffer(), handle.getOffset(), vert, 0, handle.getRange());

	TransferGpuAllocatorHandle handle2;
	ANKI_CHECK(m_r->getResourceManager().getTransferGpuAllocator().allocate(loader.getIndexDataSize(), handle2));
	void* cpuIds = handle2.getMappedMemory();

	memcpy(cpuIds, loader.getIndexData(), loader.getIndexDataSize());

	cmdb->copyBufferToBuffer(handle2.getBuffer(), handle2.getOffset(), idx, 0, handle2.getRange());
	idxCount = loader.getHeader().m_totalIndicesCount;

	FencePtr fence;
	cmdb->flush(&fence);
	m_r->getResourceManager().getTransferGpuAllocator().release(handle, fence);
	m_r->getResourceManager().getTransferGpuAllocator().release(handle2, fence);

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
			SamplingFilter::NEAREST, // Because we don't want the light pass to bleed to near faces
			"GI_gbuff");

		// Create color RT descriptions
		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			texinit.m_format = MS_COLOR_ATTACHMENT_PIXEL_FORMATS[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_usage |= TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
		texinit.m_format = GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT;
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
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
				| TextureUsageBit::GENERATE_MIPMAPS,
			SamplingFilter::LINEAR,
			"GI refl");
		texinit.m_mipmapsCount = m_lightShading.m_mipCount;
		texinit.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;
		texinit.m_type = TextureType::CUBE_ARRAY;
		texinit.m_layerCount = m_cacheEntries.getSize();
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		texinit.m_usageWhenEncountered = TextureUsageBit::SAMPLED_FRAGMENT;

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

	// Init vert/idx buffers
	ANKI_CHECK(loadMesh("engine_data/Plight.ankimesh",
		m_lightShading.m_plightPositions,
		m_lightShading.m_plightIndices,
		m_lightShading.m_plightIdxCount));

	ANKI_CHECK(loadMesh("engine_data/Slight.ankimesh",
		m_lightShading.m_slightPositions,
		m_lightShading.m_slightIndices,
		m_lightShading.m_slightIdxCount));

	return Error::NONE;
}

Error Indirect::initIrradiance(const ConfigSet& config)
{
	// Init atlas
	{
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(m_irradiance.m_tileSize,
			m_irradiance.m_tileSize,
			LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			SamplingFilter::LINEAR,
			"GI irr");

		texinit.m_layerCount = m_cacheEntries.getSize();
		texinit.m_type = TextureType::CUBE_ARRAY;
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		texinit.m_usageWhenEncountered = TextureUsageBit::SAMPLED_FRAGMENT;

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
		cmdb->setViewport(viewportX, 0, viewportX + m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);
		cmdb->setScissor(viewportX, 0, viewportX + m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);

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
	cmdb->setScissor(0, 0, MAX_U16, MAX_U16);
}

void Indirect::runLightShading(CommandBufferPtr& cmdb, const RenderGraph& rgraph, U32 faceIdx)
{
	ANKI_ASSERT(faceIdx <= 6);
	ANKI_TRACE_SCOPED_EVENT(RENDER_IR);

	ANKI_ASSERT(m_ctx.m_probe);
	const ReflectionProbeQueueElement& probe = *m_ctx.m_probe;

	// Set common state for all lights
	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
	{
		cmdb->bindTexture(0, i, rgraph.getTexture(m_ctx.m_gbufferColorRts[i]));
	}
	cmdb->bindTexture(0, GBUFFER_COLOR_ATTACHMENT_COUNT, rgraph.getTexture(m_ctx.m_gbufferDepthRt));
	cmdb->setVertexAttribute(0, 0, PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT), 0);
	cmdb->setViewport(0, 0, m_lightShading.m_tileSize, m_lightShading.m_tileSize);
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->setCullMode(FaceSelectionBit::FRONT);

	// Render lights
	{
		ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
		const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

		const Mat4& vpMat = rqueue.m_viewProjectionMatrix;
		const Mat4& vMat = rqueue.m_viewMatrix;

		// Do point lights
		cmdb->bindShaderProgram(m_lightShading.m_plightGrProg);
		cmdb->bindVertexBuffer(0, m_lightShading.m_plightPositions, 0, sizeof(F32) * 3);
		cmdb->bindIndexBuffer(m_lightShading.m_plightIndices, 0, IndexType::U16);

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

			Vec4 pos = vMat * plightEl->m_worldPosition.xyz1();

			light->m_inputTexUvScaleAndOffset = Vec4(1.0f / 6.0f, 1.0f, faceIdx * (1.0f / 6.0f), 0.0f);
			light->m_projectionParams = rqueue.m_projectionMatrix.extractPerspectiveUnprojectionParams();
			light->m_posRadius = Vec4(pos.xyz(), 1.0f / (plightEl->m_radius * plightEl->m_radius));
			light->m_diffuseColorPad1 = plightEl->m_diffuseColor.xyz0();
			light->m_specularColorPad1 = plightEl->m_specularColor.xyz0();

			// Draw
			cmdb->drawElements(PrimitiveTopology::TRIANGLES, m_lightShading.m_plightIdxCount);

			++plightEl;
		}

		// Do spot lights
		cmdb->bindShaderProgram(m_lightShading.m_slightGrProg);
		cmdb->bindVertexBuffer(0, m_lightShading.m_slightPositions, 0, sizeof(F32) * 3);
		cmdb->bindIndexBuffer(m_lightShading.m_slightIndices, 0, IndexType::U16);

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
			light->m_projectionParams = rqueue.m_projectionMatrix.extractPerspectiveUnprojectionParams();

			Vec4 pos = vMat * splightEl->m_worldTransform.getTranslationPart().xyz1();
			light->m_posRadius = Vec4(pos.xyz(), 1.0f / (splightEl->m_distance * splightEl->m_distance));

			light->m_diffuseColorOuterCos = Vec4(splightEl->m_diffuseColor, cos(splightEl->m_outerAngle / 2.0f));

			light->m_specularColorInnerCos = Vec4(splightEl->m_specularColor, cos(splightEl->m_innerAngle / 2.0f));

			Vec3 lightDir = -splightEl->m_worldTransform.getZAxis().xyz();
			lightDir = vMat.getRotationPart() * lightDir;
			light->m_lightDirPad1 = lightDir.xyz0();

			// Draw
			cmdb->drawElements(PrimitiveTopology::TRIANGLES, m_lightShading.m_slightIdxCount);

			++splightEl;
		}
	}

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setCullMode(FaceSelectionBit::BACK);
}

void Indirect::runMipmappingOfLightShading(CommandBufferPtr& cmdb, const RenderGraph& rgraph, U32 faceIdx)
{
	ANKI_ASSERT(faceIdx < 6);
	ANKI_ASSERT(m_ctx.m_cacheEntryIdx < m_cacheEntries.getSize());

	ANKI_TRACE_SCOPED_EVENT(RENDER_IR);
	cmdb->generateMipmaps2d(rgraph.getTexture(m_ctx.m_lightShadingRt), faceIdx, m_ctx.m_cacheEntryIdx);
}

void Indirect::runIrradiance(CommandBufferPtr& cmdb, const RenderGraph& rgraph, U32 faceIdx)
{
	ANKI_ASSERT(faceIdx < 6);
	ANKI_TRACE_SCOPED_EVENT(RENDER_IR);
	const U32 cacheEntryIdx = m_ctx.m_cacheEntryIdx;
	ANKI_ASSERT(cacheEntryIdx < m_cacheEntries.getSize());

	// Set state
	cmdb->bindShaderProgram(m_irradiance.m_grProg);
	cmdb->bindTexture(0, 0, rgraph.getTexture(m_ctx.m_lightShadingRt));
	cmdb->setViewport(0, 0, m_irradiance.m_tileSize, m_irradiance.m_tileSize);

	// Set uniforms
	UVec4* faceIdxArrayIdx = allocateAndBindUniforms<UVec4*>(sizeof(UVec4), cmdb, 0, 0);
	faceIdxArrayIdx->x() = faceIdx;
	faceIdxArrayIdx->y() = cacheEntryIdx;

	// Draw
	m_r->drawQuad(cmdb);
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
			pass.newConsumer({m_ctx.m_gbufferDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE});
			pass.newProducer({m_ctx.m_gbufferDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE});
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
			for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
			{
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI lightshad");
				pass.setFramebufferInfo(m_cacheEntries[probeToUpdateCacheEntryIdx].m_lightShadingFbDescrs[faceIdx],
					{{m_ctx.m_lightShadingRt}},
					{});
				pass.setWork(callbacks[faceIdx], this, 0);

				TextureSurfaceInfo surf(0, 0, faceIdx, probeToUpdateCacheEntryIdx);
				pass.newConsumer({m_ctx.m_lightShadingRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf});
				pass.newProducer({m_ctx.m_lightShadingRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf});

				for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
				{
					pass.newConsumer({m_ctx.m_gbufferColorRts[i], TextureUsageBit::SAMPLED_FRAGMENT});
				}
				pass.newConsumer({m_ctx.m_gbufferDepthRt, TextureUsageBit::SAMPLED_FRAGMENT});
			}
		}

		// Mipmapping "passes"
		{
			Array<RenderPassWorkCallback, 6> callbacks = {{runMipmappingOfLightShadingCallback<0>,
				runMipmappingOfLightShadingCallback<1>,
				runMipmappingOfLightShadingCallback<2>,
				runMipmappingOfLightShadingCallback<3>,
				runMipmappingOfLightShadingCallback<4>,
				runMipmappingOfLightShadingCallback<5>}};

			for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
			{
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI mipmap");
				pass.setWork(callbacks[faceIdx], this, 0);

				for(U mip = 0; mip < m_lightShading.m_mipCount; ++mip)
				{
					TextureSurfaceInfo surf(mip, 0, faceIdx, probeToUpdateCacheEntryIdx);
					pass.newConsumer({m_ctx.m_lightShadingRt, TextureUsageBit::GENERATE_MIPMAPS, surf});
					pass.newProducer({m_ctx.m_lightShadingRt, TextureUsageBit::GENERATE_MIPMAPS, surf});
				}
			}
		}

		// Irradiance passes
		{
			Array<RenderPassWorkCallback, 6> callbacks = {{runIrradianceCallback<0>,
				runIrradianceCallback<1>,
				runIrradianceCallback<2>,
				runIrradianceCallback<3>,
				runIrradianceCallback<4>,
				runIrradianceCallback<5>}};

			// Rt
			m_ctx.m_irradianceRt =
				rgraph.importRenderTarget("GI irradiance", m_irradiance.m_cubeArr, TextureUsageBit::SAMPLED_FRAGMENT);

			for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
			{
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI irradiance");

				pass.setFramebufferInfo(m_cacheEntries[probeToUpdateCacheEntryIdx].m_irradianceFbDescrs[faceIdx],
					{{m_ctx.m_irradianceRt}},
					{});

				pass.setWork(callbacks[faceIdx], this, 0);

				pass.newConsumer({m_ctx.m_irradianceRt,
					TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
					TextureSurfaceInfo(0, 0, faceIdx, probeToUpdateCacheEntryIdx)});
				pass.newProducer({m_ctx.m_irradianceRt,
					TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
					TextureSurfaceInfo(0, 0, faceIdx, probeToUpdateCacheEntryIdx)});

				pass.newConsumer({m_ctx.m_lightShadingRt, TextureUsageBit::SAMPLED_FRAGMENT});
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
