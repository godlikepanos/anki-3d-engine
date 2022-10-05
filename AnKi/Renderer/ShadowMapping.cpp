// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

class ShadowMapping::Scratch::WorkItem
{
public:
	UVec4 m_viewport;
	RenderQueue* m_renderQueue;
	U32 m_firstRenderableElement;
	U32 m_renderableElementCount;
	U32 m_threadPoolTaskIdx;
	U32 m_renderQueueElementsLod;
};

class ShadowMapping::Scratch::LightToRenderToScratchInfo
{
public:
	UVec4 m_viewport;
	RenderQueue* m_renderQueue;
	U32 m_drawcallCount;
	U32 m_renderQueueElementsLod;
};

class ShadowMapping::Atlas::ResolveWorkItem
{
public:
	Vec4 m_uvInBounds; ///< Bounds used to avoid blurring neighbour tiles.
	Vec4 m_uvIn; ///< UV + size that point to the scratch buffer.
	UVec4 m_viewportOut; ///< Viewport in the atlas RT.
	Bool m_blur;
};

ShadowMapping::~ShadowMapping()
{
}

Error ShadowMapping::init()
{
	ANKI_R_LOGV("Initializing shadowmapping")

	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadowmapping");
	}
	else
	{
		ANKI_R_LOGV("Shadowmapping initialized. Scratch size %ux%u, atlas size %ux%u",
					m_scratch.m_tileCountX * m_scratch.m_tileResolution,
					m_scratch.m_tileCountY * m_scratch.m_tileResolution,
					m_atlas.m_tileCountBothAxis * m_atlas.m_tileResolution,
					m_atlas.m_tileCountBothAxis * m_atlas.m_tileResolution);
	}

	return err;
}

Error ShadowMapping::initScratch()
{
	// Init the shadowmaps and FBs
	{
		m_scratch.m_tileCountX = getConfig().getRShadowMappingScratchTileCountX();
		m_scratch.m_tileCountY = getConfig().getRShadowMappingScratchTileCountY();
		m_scratch.m_tileResolution = getConfig().getRShadowMappingTileResolution();

		// RT
		m_scratch.m_rtDescr = m_r->create2DRenderTargetDescription(m_scratch.m_tileResolution * m_scratch.m_tileCountX,
																   m_scratch.m_tileResolution * m_scratch.m_tileCountY,
																   m_r->getDepthNoStencilFormat(), "SM scratch");
		m_scratch.m_rtDescr.bake();

		// FB
		m_scratch.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kClear;
		m_scratch.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
		m_scratch.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
		m_scratch.m_fbDescr.bake();
	}

	m_scratch.m_tileAlloc.init(&getMemoryPool(), m_scratch.m_tileCountX, m_scratch.m_tileCountY, kMaxLodCount, false);

	return Error::kNone;
}

Error ShadowMapping::initAtlas()
{
	const Bool preferCompute = getConfig().getRPreferCompute();

	// Init RT
	{
		m_atlas.m_tileResolution = getConfig().getRShadowMappingTileResolution();
		m_atlas.m_tileCountBothAxis = getConfig().getRShadowMappingTileCountPerRowOrColumn();

		// RT
		const Format texFormat = (ANKI_EVSM4) ? Format::kR32G32B32A32_Sfloat : Format::kR32G32_Sfloat;
		TextureUsageBit usage = TextureUsageBit::kSampledFragment | TextureUsageBit::kSampledCompute;
		usage |= (preferCompute) ? TextureUsageBit::kImageComputeWrite : TextureUsageBit::kAllFramebuffer;
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(
			m_atlas.m_tileResolution * m_atlas.m_tileCountBothAxis,
			m_atlas.m_tileResolution * m_atlas.m_tileCountBothAxis, texFormat, usage, "SM atlas");
		ClearValue clearVal;
		clearVal.m_colorf[0] = 1.0f;
		m_atlas.m_tex = m_r->createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment, clearVal);
	}

	// Tiles
	m_atlas.m_tileAlloc.init(&getMemoryPool(), m_atlas.m_tileCountBothAxis, m_atlas.m_tileCountBothAxis, kMaxLodCount,
							 true);

	// Programs and shaders
	{
		ANKI_CHECK(getResourceManager().loadResource((preferCompute) ? "ShaderBinaries/EvsmCompute.ankiprogbin"
																	 : "ShaderBinaries/EvsmRaster.ankiprogbin",
													 m_atlas.m_resolveProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_atlas.m_resolveProg);
		variantInitInfo.addConstant("kInputTextureSize", UVec2(m_scratch.m_tileCountX * m_scratch.m_tileResolution,
															   m_scratch.m_tileCountY * m_scratch.m_tileResolution));

		if(!preferCompute)
		{
			variantInitInfo.addConstant("kFramebufferSize",
										UVec2(m_atlas.m_tileCountBothAxis * m_atlas.m_tileResolution));
		}

		const ShaderProgramResourceVariant* variant;
		m_atlas.m_resolveProg->getOrCreateVariant(variantInitInfo, variant);
		m_atlas.m_resolveGrProg = variant->getProgram();
	}

	m_atlas.m_fbDescr.m_colorAttachmentCount = 1;
	m_atlas.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kLoad;
	m_atlas.m_fbDescr.bake();

	return Error::kNone;
}

Error ShadowMapping::initInternal()
{
	ANKI_CHECK(initScratch());
	ANKI_CHECK(initAtlas());
	return Error::kNone;
}

void ShadowMapping::runAtlas(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_atlas.m_resolveWorkItems.getSize());
	ANKI_TRACE_SCOPED_EVENT(R_SM);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	// Allocate and populate uniforms
	EvsmResolveUniforms* uniforms = allocateAndBindStorage<EvsmResolveUniforms*>(
		m_atlas.m_resolveWorkItems.getSize() * sizeof(EvsmResolveUniforms), cmdb, 0, 0);
	for(U32 i = 0; i < m_atlas.m_resolveWorkItems.getSize(); ++i)
	{
		EvsmResolveUniforms& uni = uniforms[i];
		const Atlas::ResolveWorkItem& workItem = m_atlas.m_resolveWorkItems[i];

		uni.m_viewportXY = IVec2(workItem.m_viewportOut.xy());
		uni.m_viewportZW = Vec2(workItem.m_viewportOut.zw());

		uni.m_uvScale = workItem.m_uvIn.zw();
		uni.m_uvTranslation = workItem.m_uvIn.xy();

		uni.m_uvMin = workItem.m_uvInBounds.xy();
		uni.m_uvMax = workItem.m_uvInBounds.xy() + workItem.m_uvInBounds.zw();

		uni.m_blur = workItem.m_blur;
	}

	cmdb->bindShaderProgram(m_atlas.m_resolveGrProg);

	// Continue
	cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindTexture(0, 2, m_scratch.m_rt, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

	if(getConfig().getRPreferCompute())
	{
		rgraphCtx.bindImage(0, 3, m_atlas.m_rt);

		constexpr U32 workgroupSize = 8;
		ANKI_ASSERT(m_atlas.m_tileResolution >= workgroupSize && (m_atlas.m_tileResolution % workgroupSize) == 0);

		cmdb->dispatchCompute(m_atlas.m_tileResolution / workgroupSize, m_atlas.m_tileResolution / workgroupSize,
							  m_atlas.m_resolveWorkItems.getSize());
	}
	else
	{
		cmdb->setViewport(0, 0, m_atlas.m_tex->getWidth(), m_atlas.m_tex->getHeight());

		cmdb->drawArrays(PrimitiveTopology::kTriangles, 6, m_atlas.m_resolveWorkItems.getSize());
	}
}

void ShadowMapping::runShadowMapping(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_scratch.m_workItems.getSize());
	ANKI_TRACE_SCOPED_EVENT(R_SM);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const U threadIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;

	for(Scratch::WorkItem& work : m_scratch.m_workItems)
	{
		if(work.m_threadPoolTaskIdx != threadIdx)
		{
			continue;
		}

		// Set state
		cmdb->setViewport(work.m_viewport[0], work.m_viewport[1], work.m_viewport[2], work.m_viewport[3]);
		cmdb->setScissor(work.m_viewport[0], work.m_viewport[1], work.m_viewport[2], work.m_viewport[3]);

		RenderableDrawerArguments args;
		args.m_viewMatrix = work.m_renderQueue->m_viewMatrix;
		args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
		args.m_viewProjectionMatrix = work.m_renderQueue->m_viewProjectionMatrix;
		args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
		args.m_sampler = m_r->getSamplers().m_trilinearRepeatAniso;
		args.m_minLod = args.m_maxLod = work.m_renderQueueElementsLod;

		m_r->getSceneDrawer().drawRange(RenderingTechnique::kShadow, args,
										work.m_renderQueue->m_renderables.getBegin() + work.m_firstRenderableElement,
										work.m_renderQueue->m_renderables.getBegin() + work.m_firstRenderableElement
											+ work.m_renderableElementCount,
										cmdb);
	}
}

void ShadowMapping::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_SM);

	// First process the lights
	U32 threadCountForScratchPass = 0;
	processLights(ctx, threadCountForScratchPass);

	// Build the render graph
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	if(m_scratch.m_workItems.getSize())
	{
		// Will have to create render passes

		// Scratch pass
		{
			// Compute render area
			const U32 minx = 0, miny = 0;
			const U32 height = m_scratch.m_maxViewportHeight;
			const U32 width = m_scratch.m_maxViewportWidth;

			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SM scratch");

			m_scratch.m_rt = rgraph.newRenderTarget(m_scratch.m_rtDescr);
			pass.setFramebufferInfo(m_scratch.m_fbDescr, {}, m_scratch.m_rt, {}, minx, miny, width, height);
			ANKI_ASSERT(threadCountForScratchPass
						&& threadCountForScratchPass <= m_r->getThreadHive().getThreadCount());
			pass.setWork(threadCountForScratchPass, [this](RenderPassWorkContext& rgraphCtx) {
				runShadowMapping(rgraphCtx);
			});

			TextureSubresourceInfo subresource = TextureSubresourceInfo(DepthStencilAspectBit::kDepth);
			pass.newTextureDependency(m_scratch.m_rt, TextureUsageBit::kAllFramebuffer, subresource);
		}

		// Atlas pass
		{
			if(ANKI_LIKELY(m_atlas.m_rtImportedOnce))
			{
				m_atlas.m_rt = rgraph.importRenderTarget(m_atlas.m_tex);
			}
			else
			{
				m_atlas.m_rt = rgraph.importRenderTarget(m_atlas.m_tex, TextureUsageBit::kSampledFragment);
				m_atlas.m_rtImportedOnce = true;
			}

			if(getConfig().getRPreferCompute())
			{
				ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("EVSM resolve");

				pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
					runAtlas(rgraphCtx);
				});

				pass.newTextureDependency(m_scratch.m_rt, TextureUsageBit::kSampledCompute,
										  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
				pass.newTextureDependency(m_atlas.m_rt, TextureUsageBit::kImageComputeWrite);
			}
			else
			{
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("EVSM resolve");
				pass.setFramebufferInfo(m_atlas.m_fbDescr, {m_atlas.m_rt});

				pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
					runAtlas(rgraphCtx);
				});

				pass.newTextureDependency(m_scratch.m_rt, TextureUsageBit::kSampledFragment,
										  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
				pass.newTextureDependency(m_atlas.m_rt,
										  TextureUsageBit::kFramebufferRead | TextureUsageBit::kFramebufferWrite);
			}
		}
	}
	else
	{
		// No need for shadowmapping passes, just import the atlas
		if(ANKI_LIKELY(m_atlas.m_rtImportedOnce))
		{
			m_atlas.m_rt = rgraph.importRenderTarget(m_atlas.m_tex);
		}
		else
		{
			m_atlas.m_rt = rgraph.importRenderTarget(m_atlas.m_tex, TextureUsageBit::kSampledFragment);
			m_atlas.m_rtImportedOnce = true;
		}
	}
}

Mat4 ShadowMapping::createSpotLightTextureMatrix(const UVec4& viewport) const
{
	const F32 atlasSize = F32(m_atlas.m_tileResolution * m_atlas.m_tileCountBothAxis);
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wpedantic" // Because GCC and clang throw an incorrect warning
#endif
	const Vec2 uv(F32(viewport[0]) / atlasSize, F32(viewport[1]) / atlasSize);
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif
	ANKI_ASSERT(uv >= Vec2(0.0f) && uv <= Vec2(1.0f));

	ANKI_ASSERT(viewport[2] == viewport[3]);
	const F32 sizeTextureSpace = F32(viewport[2]) / atlasSize;

	return Mat4(sizeTextureSpace, 0.0f, 0.0f, uv.x(), 0.0f, sizeTextureSpace, 0.0f, uv.y(), 0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
}

void ShadowMapping::chooseLod(const Vec4& cameraOrigin, const PointLightQueueElement& light, Bool& blurAtlas,
							  U32& tileBufferLod, U32& renderQueueElementsLod) const
{
	const F32 distFromTheCamera = (cameraOrigin - light.m_worldPosition.xyz0()).getLength() - light.m_radius;
	if(distFromTheCamera < getConfig().getLod0MaxDistance())
	{
		ANKI_ASSERT(m_pointLightsMaxLod == 1);
		blurAtlas = true;
		tileBufferLod = 1;
		renderQueueElementsLod = 0;
	}
	else
	{
		blurAtlas = false;
		tileBufferLod = 0;
		renderQueueElementsLod = kMaxLodCount - 1;
	}
}

void ShadowMapping::chooseLod(const Vec4& cameraOrigin, const SpotLightQueueElement& light, Bool& blurAtlas,
							  U32& tileBufferLod, U32& renderQueueElementsLod) const
{
	// Get some data
	const Vec4 coneOrigin = light.m_worldTransform.getTranslationPart().xyz0();
	const Vec4 coneDir = -light.m_worldTransform.getZAxis().xyz0();
	const F32 coneAngle = light.m_outerAngle;

	// Compute the distance from the camera to the light cone
	const Vec4 V = cameraOrigin - coneOrigin;
	const F32 VlenSq = V.dot(V);
	const F32 V1len = V.dot(coneDir);
	const F32 distFromTheCamera = cos(coneAngle) * sqrt(VlenSq - V1len * V1len) - V1len * sin(coneAngle);

	if(distFromTheCamera < getConfig().getLod0MaxDistance())
	{
		blurAtlas = true;
		tileBufferLod = 2;
		renderQueueElementsLod = 0;
	}
	else if(distFromTheCamera < getConfig().getLod1MaxDistance())
	{
		blurAtlas = false;
		tileBufferLod = 1;
		renderQueueElementsLod = kMaxLodCount - 1;
	}
	else
	{
		blurAtlas = false;
		tileBufferLod = 0;
		renderQueueElementsLod = kMaxLodCount - 1;
	}
}

TileAllocatorResult ShadowMapping::allocateTilesAndScratchTiles(U64 lightUuid, U32 faceCount, const U64* faceTimestamps,
																const U32* faceIndices, const U32* drawcallsCount,
																const U32* lods, UVec4* atlasTileViewports,
																UVec4* scratchTileViewports,
																TileAllocatorResult* subResults)
{
	ANKI_ASSERT(lightUuid > 0);
	ANKI_ASSERT(faceCount > 0);
	ANKI_ASSERT(faceTimestamps);
	ANKI_ASSERT(faceIndices);
	ANKI_ASSERT(drawcallsCount);
	ANKI_ASSERT(lods);

	TileAllocatorResult res = TileAllocatorResult::kAllocationFailed;

	// Allocate atlas tiles first. They may be cached and that will affect how many scratch tiles we'll need
	for(U i = 0; i < faceCount; ++i)
	{
		Array<U32, 4> tileRanges;
		res = m_atlas.m_tileAlloc.allocate(m_r->getGlobalTimestamp(), faceTimestamps[i], lightUuid, faceIndices[i],
										   drawcallsCount[i], lods[i], tileRanges);

		if(res == TileAllocatorResult::kAllocationFailed)
		{
			ANKI_R_LOGW("There is not enough space in the shadow atlas for more shadow maps. "
						"Increase the RShadowMappingTileCountPerRowOrColumn or decrease the scene's shadow casters");

			// Invalidate cache entries for what we already allocated
			for(U j = 0; j < i; ++j)
			{
				m_atlas.m_tileAlloc.invalidateCache(lightUuid, faceIndices[j]);
			}

			return res;
		}

		subResults[i] = res;

		// Set viewport
		atlasTileViewports[i] = UVec4(tileRanges) * m_atlas.m_tileResolution;
	}

	// Allocate scratch tiles
	for(U i = 0; i < faceCount; ++i)
	{
		if(subResults[i] == TileAllocatorResult::kCached)
		{
			continue;
		}

		ANKI_ASSERT(subResults[i] == TileAllocatorResult::kAllocationSucceded);

		Array<U32, 4> tileRanges;
		res = m_scratch.m_tileAlloc.allocate(m_r->getGlobalTimestamp(), faceTimestamps[i], lightUuid, faceIndices[i],
											 drawcallsCount[i], lods[i], tileRanges);

		if(res == TileAllocatorResult::kAllocationFailed)
		{
			ANKI_R_LOGW("Don't have enough space in the scratch shadow mapping buffer. "
						"If you see this message too often increase RShadowMappingScratchTileCountX/Y");

			// Invalidate atlas tiles
			for(U j = 0; j < faceCount; ++j)
			{
				m_atlas.m_tileAlloc.invalidateCache(lightUuid, faceIndices[j]);
			}

			return res;
		}

		// Fix viewport
		scratchTileViewports[i] = UVec4(tileRanges) * m_scratch.m_tileResolution;

		// Update the max view width
		m_scratch.m_maxViewportWidth =
			max(m_scratch.m_maxViewportWidth, scratchTileViewports[i][0] + scratchTileViewports[i][2]);
		m_scratch.m_maxViewportHeight =
			max(m_scratch.m_maxViewportHeight, scratchTileViewports[i][1] + scratchTileViewports[i][3]);
	}

	return res;
}

void ShadowMapping::processLights(RenderingContext& ctx, U32& threadCountForScratchPass)
{
	// Reset the scratch viewport width
	m_scratch.m_maxViewportWidth = 0;
	m_scratch.m_maxViewportHeight = 0;

	// Vars
	const Vec4 cameraOrigin = ctx.m_renderQueue->m_cameraTransform.getTranslationPart().xyz0();
	DynamicArrayRaii<Scratch::LightToRenderToScratchInfo> lightsToRender(ctx.m_tempPool);
	U32 drawcallCount = 0;
	DynamicArrayRaii<Atlas::ResolveWorkItem> atlasWorkItems(ctx.m_tempPool);

	// First thing, allocate an empty tile for empty faces of point lights
	UVec4 emptyTileViewport;
	{
		Array<U32, 4> tileRange;
		[[maybe_unused]] const TileAllocatorResult res =
			m_atlas.m_tileAlloc.allocate(m_r->getGlobalTimestamp(), 1, kMaxU64, 0, 1, m_pointLightsMaxLod, tileRange);

		emptyTileViewport = UVec4(tileRange);

#if ANKI_ENABLE_ASSERTIONS
		static Bool firstRun = true;
		if(firstRun)
		{
			ANKI_ASSERT(res == TileAllocatorResult::kAllocationSucceded);
			firstRun = false;
		}
		else
		{
			ANKI_ASSERT(res == TileAllocatorResult::kCached);
		}
#endif
	}

	// Process the directional light first.
	if(ctx.m_renderQueue->m_directionalLight.m_shadowCascadeCount > 0)
	{
		DirectionalLightQueueElement& light = ctx.m_renderQueue->m_directionalLight;

		Array<U64, kMaxShadowCascades> timestamps;
		Array<U32, kMaxShadowCascades> cascadeIndices;
		Array<U32, kMaxShadowCascades> drawcallCounts;
		Array<UVec4, kMaxShadowCascades> atlasViewports;
		Array<UVec4, kMaxShadowCascades> scratchViewports;
		Array<TileAllocatorResult, kMaxShadowCascades> subResults;
		Array<U32, kMaxShadowCascades> lods;
		Array<U32, kMaxShadowCascades> renderQueueElementsLods;
		Array<Bool, kMaxShadowCascades> blurAtlass;

		U32 activeCascades = 0;

		for(U32 cascade = 0; cascade < light.m_shadowCascadeCount; ++cascade)
		{
			ANKI_ASSERT(light.m_shadowRenderQueues[cascade]);
			if(light.m_shadowRenderQueues[cascade]->m_renderables.getSize() > 0)
			{
				// Cascade with drawcalls, will need tiles

				timestamps[activeCascades] = m_r->getGlobalTimestamp(); // This light is always updated
				cascadeIndices[activeCascades] = cascade;
				drawcallCounts[activeCascades] = 1; // Doesn't matter

				// Change the quality per cascade
				blurAtlass[activeCascades] = (cascade <= 1);
				lods[activeCascades] = (cascade <= 1) ? (kMaxLodCount - 1) : (lods[0] - 1);
				renderQueueElementsLods[activeCascades] = (cascade == 0) ? 0 : (kMaxLodCount - 1);

				++activeCascades;
			}
		}

		const Bool allocationFailed =
			activeCascades == 0
			|| allocateTilesAndScratchTiles(light.m_uuid, activeCascades, &timestamps[0], &cascadeIndices[0],
											&drawcallCounts[0], &lods[0], &atlasViewports[0], &scratchViewports[0],
											&subResults[0])
				   == TileAllocatorResult::kAllocationFailed;

		if(!allocationFailed)
		{
			activeCascades = 0;

			for(U cascade = 0; cascade < light.m_shadowCascadeCount; ++cascade)
			{
				if(light.m_shadowRenderQueues[cascade]->m_renderables.getSize() > 0)
				{
					// Cascade with drawcalls, push some work for it

					// Update the texture matrix to point to the correct region in the atlas
					light.m_textureMatrices[cascade] =
						createSpotLightTextureMatrix(atlasViewports[activeCascades]) * light.m_textureMatrices[cascade];

					// Push work
					newScratchAndAtlasResloveRenderWorkItems(
						atlasViewports[activeCascades], scratchViewports[activeCascades], blurAtlass[activeCascades],
						light.m_shadowRenderQueues[cascade], renderQueueElementsLods[activeCascades], lightsToRender,
						atlasWorkItems, drawcallCount);

					++activeCascades;
				}
				else
				{
					// Empty cascade, point it to the empty tile

					light.m_textureMatrices[cascade] =
						createSpotLightTextureMatrix(emptyTileViewport) * light.m_textureMatrices[cascade];
				}
			}
		}
		else
		{
			// Light can't be a caster this frame
			light.m_shadowCascadeCount = 0;
			zeroMemory(light.m_shadowRenderQueues);
		}
	}

	// Process the point lights.
	for(PointLightQueueElement& light : ctx.m_renderQueue->m_pointLights)
	{
		if(!light.hasShadow())
		{
			continue;
		}

		// Prepare data to allocate tiles and allocate
		Array<U64, 6> timestamps;
		Array<U32, 6> faceIndices;
		Array<U32, 6> drawcallCounts;
		Array<UVec4, 6> atlasViewports;
		Array<UVec4, 6> scratchViewports;
		Array<TileAllocatorResult, 6> subResults;
		Array<U32, 6> lods;
		U32 numOfFacesThatHaveDrawcalls = 0;

		Bool blurAtlas;
		U32 lod, renderQueueElementsLod;
		chooseLod(cameraOrigin, light, blurAtlas, lod, renderQueueElementsLod);

		for(U32 face = 0; face < 6; ++face)
		{
			ANKI_ASSERT(light.m_shadowRenderQueues[face]);
			if(light.m_shadowRenderQueues[face]->m_renderables.getSize())
			{
				// Has renderables, need to allocate tiles for it so add it to the arrays

				faceIndices[numOfFacesThatHaveDrawcalls] = face;
				timestamps[numOfFacesThatHaveDrawcalls] =
					light.m_shadowRenderQueues[face]->m_shadowRenderablesLastUpdateTimestamp;

				drawcallCounts[numOfFacesThatHaveDrawcalls] = light.m_shadowRenderQueues[face]->m_renderables.getSize();

				lods[numOfFacesThatHaveDrawcalls] = lod;

				++numOfFacesThatHaveDrawcalls;
			}
		}

		const Bool allocationFailed =
			numOfFacesThatHaveDrawcalls == 0
			|| allocateTilesAndScratchTiles(light.m_uuid, numOfFacesThatHaveDrawcalls, &timestamps[0], &faceIndices[0],
											&drawcallCounts[0], &lods[0], &atlasViewports[0], &scratchViewports[0],
											&subResults[0])
				   == TileAllocatorResult::kAllocationFailed;

		if(!allocationFailed)
		{
			// All good, update the lights

			const F32 atlasResolution = F32(m_atlas.m_tileResolution * m_atlas.m_tileCountBothAxis);
			F32 superTileSize = F32(atlasViewports[0][2]); // Should be the same for all tiles and faces
			superTileSize -= 1.0f; // Remove 2 half texels to avoid bilinear filtering bleeding

			light.m_shadowAtlasTileSize = superTileSize / atlasResolution;

			numOfFacesThatHaveDrawcalls = 0;
			for(U face = 0; face < 6; ++face)
			{
				if(light.m_shadowRenderQueues[face]->m_renderables.getSize())
				{
					// Has drawcalls, asigned it to a tile

					const UVec4& atlasViewport = atlasViewports[numOfFacesThatHaveDrawcalls];
					const UVec4& scratchViewport = scratchViewports[numOfFacesThatHaveDrawcalls];

					// Add a half texel to the viewport's start to avoid bilinear filtering bleeding
					light.m_shadowAtlasTileOffsets[face].x() = (F32(atlasViewport[0]) + 0.5f) / atlasResolution;
					light.m_shadowAtlasTileOffsets[face].y() = (F32(atlasViewport[1]) + 0.5f) / atlasResolution;

					if(subResults[numOfFacesThatHaveDrawcalls] != TileAllocatorResult::kCached)
					{
						newScratchAndAtlasResloveRenderWorkItems(
							atlasViewport, scratchViewport, blurAtlas, light.m_shadowRenderQueues[face],
							renderQueueElementsLod, lightsToRender, atlasWorkItems, drawcallCount);
					}

					++numOfFacesThatHaveDrawcalls;
				}
				else
				{
					// Doesn't have renderables, point the face to the empty tile
					UVec4 atlasViewport = emptyTileViewport;
					ANKI_ASSERT(F32(atlasViewport[2]) <= superTileSize && F32(atlasViewport[3]) <= superTileSize);
					atlasViewport[2] = U32(superTileSize);
					atlasViewport[3] = U32(superTileSize);

					light.m_shadowAtlasTileOffsets[face].x() = (F32(atlasViewport[0]) + 0.5f) / atlasResolution;
					light.m_shadowAtlasTileOffsets[face].y() = (F32(atlasViewport[1]) + 0.5f) / atlasResolution;
				}
			}
		}
		else
		{
			// Light can't be a caster this frame
			zeroMemory(light.m_shadowRenderQueues);
		}
	}

	// Process the spot lights
	for(SpotLightQueueElement& light : ctx.m_renderQueue->m_spotLights)
	{
		if(!light.hasShadow())
		{
			continue;
		}

		// Allocate tiles
		U32 faceIdx = 0;
		TileAllocatorResult subResult = TileAllocatorResult::kAllocationFailed;
		UVec4 atlasViewport;
		UVec4 scratchViewport;
		const U32 localDrawcallCount = light.m_shadowRenderQueue->m_renderables.getSize();

		Bool blurAtlas;
		U32 lod, renderQueueElementsLod;
		chooseLod(cameraOrigin, light, blurAtlas, lod, renderQueueElementsLod);

		const Bool allocationFailed =
			localDrawcallCount == 0
			|| allocateTilesAndScratchTiles(
				   light.m_uuid, 1, &light.m_shadowRenderQueue->m_shadowRenderablesLastUpdateTimestamp, &faceIdx,
				   &localDrawcallCount, &lod, &atlasViewport, &scratchViewport, &subResult)
				   == TileAllocatorResult::kAllocationFailed;

		if(!allocationFailed)
		{
			// All good, update the light

			// Update the texture matrix to point to the correct region in the atlas
			light.m_textureMatrix = createSpotLightTextureMatrix(atlasViewport) * light.m_textureMatrix;

			if(subResult != TileAllocatorResult::kCached)
			{
				newScratchAndAtlasResloveRenderWorkItems(atlasViewport, scratchViewport, blurAtlas,
														 light.m_shadowRenderQueue, renderQueueElementsLod,
														 lightsToRender, atlasWorkItems, drawcallCount);
			}
		}
		else
		{
			// Doesn't have renderables or the allocation failed, won't be a shadow caster
			light.m_shadowRenderQueue = nullptr;
		}
	}

	// Split the work that will happen in the scratch buffer
	if(lightsToRender.getSize())
	{
		DynamicArrayRaii<Scratch::WorkItem> workItems(ctx.m_tempPool);
		Scratch::LightToRenderToScratchInfo* lightToRender = lightsToRender.getBegin();
		U32 lightToRenderDrawcallCount = lightToRender->m_drawcallCount;
		const Scratch::LightToRenderToScratchInfo* lightToRenderEnd = lightsToRender.getEnd();

		const U32 threadCount = computeNumberOfSecondLevelCommandBuffers(drawcallCount);
		threadCountForScratchPass = threadCount;
		for(U32 taskId = 0; taskId < threadCount; ++taskId)
		{
			U32 start, end;
			splitThreadedProblem(taskId, threadCount, drawcallCount, start, end);

			// While there are drawcalls in this task emit new work items
			U32 taskDrawcallCount = end - start;
			ANKI_ASSERT(taskDrawcallCount > 0 && "Because we used computeNumberOfSecondLevelCommandBuffers()");

			while(taskDrawcallCount)
			{
				ANKI_ASSERT(lightToRender != lightToRenderEnd);
				const U32 workItemDrawcallCount = min(lightToRenderDrawcallCount, taskDrawcallCount);

				Scratch::WorkItem workItem;
				workItem.m_viewport = lightToRender->m_viewport;
				workItem.m_renderQueue = lightToRender->m_renderQueue;
				workItem.m_firstRenderableElement = lightToRender->m_drawcallCount - lightToRenderDrawcallCount;
				workItem.m_renderableElementCount = workItemDrawcallCount;
				workItem.m_threadPoolTaskIdx = taskId;
				workItem.m_renderQueueElementsLod = lightToRender->m_renderQueueElementsLod;
				workItems.emplaceBack(workItem);

				// Decrease the drawcall counts for the task and the light
				ANKI_ASSERT(taskDrawcallCount >= workItemDrawcallCount);
				taskDrawcallCount -= workItemDrawcallCount;
				ANKI_ASSERT(lightToRenderDrawcallCount >= workItemDrawcallCount);
				lightToRenderDrawcallCount -= workItemDrawcallCount;

				// Move to the next light
				if(lightToRenderDrawcallCount == 0)
				{
					++lightToRender;
					lightToRenderDrawcallCount =
						(lightToRender != lightToRenderEnd) ? lightToRender->m_drawcallCount : 0;
				}
			}
		}

		ANKI_ASSERT(lightToRender == lightToRenderEnd);
		ANKI_ASSERT(lightsToRender.getSize() <= workItems.getSize());

		// All good, store the work items for the threads to pick up
		{
			Scratch::WorkItem* items;
			U32 itemSize;
			U32 itemStorageSize;
			workItems.moveAndReset(items, itemSize, itemStorageSize);

			ANKI_ASSERT(items && itemSize && itemStorageSize);
			m_scratch.m_workItems = WeakArray<Scratch::WorkItem>(items, itemSize);

			Atlas::ResolveWorkItem* atlasItems;
			atlasWorkItems.moveAndReset(atlasItems, itemSize, itemStorageSize);
			ANKI_ASSERT(atlasItems && itemSize && itemStorageSize);
			m_atlas.m_resolveWorkItems = WeakArray<Atlas::ResolveWorkItem>(atlasItems, itemSize);
		}
	}
	else
	{
		m_scratch.m_workItems = WeakArray<Scratch::WorkItem>();
		m_atlas.m_resolveWorkItems = WeakArray<Atlas::ResolveWorkItem>();
	}
}

void ShadowMapping::newScratchAndAtlasResloveRenderWorkItems(
	const UVec4& atlasViewport, const UVec4& scratchVewport, Bool blurAtlas, RenderQueue* lightRenderQueue,
	U32 renderQueueElementsLod, DynamicArrayRaii<Scratch::LightToRenderToScratchInfo>& scratchWorkItem,
	DynamicArrayRaii<Atlas::ResolveWorkItem>& atlasResolveWorkItem, U32& drawcallCount) const
{
	// Scratch work item
	{
		Scratch::LightToRenderToScratchInfo toRender;
		toRender.m_renderQueue = lightRenderQueue;
		toRender.m_viewport = scratchVewport;
		toRender.m_drawcallCount = lightRenderQueue->m_renderables.getSize();
		toRender.m_renderQueueElementsLod = renderQueueElementsLod;

		scratchWorkItem.emplaceBack(toRender);
		drawcallCount += lightRenderQueue->m_renderables.getSize();
	}

	// Atlas resolve work items
	const U32 tilesX = scratchVewport[2] / m_scratch.m_tileResolution;
	const U32 tilesY = scratchVewport[3] / m_scratch.m_tileResolution;
	for(U32 x = 0; x < tilesX; ++x)
	{
		for(U32 y = 0; y < tilesY; ++y)
		{
			const F32 scratchAtlasWidth = F32(m_scratch.m_tileCountX * m_scratch.m_tileResolution);
			const F32 scratchAtlasHeight = F32(m_scratch.m_tileCountY * m_scratch.m_tileResolution);

			Atlas::ResolveWorkItem atlasItem;

			atlasItem.m_uvInBounds[0] = F32(scratchVewport[0]) / scratchAtlasWidth;
			atlasItem.m_uvInBounds[1] = F32(scratchVewport[1]) / scratchAtlasHeight;
			atlasItem.m_uvInBounds[2] = F32(scratchVewport[2]) / scratchAtlasWidth;
			atlasItem.m_uvInBounds[3] = F32(scratchVewport[3]) / scratchAtlasHeight;

			atlasItem.m_uvIn[0] = F32(scratchVewport[0] + scratchVewport[2] / tilesX * x) / scratchAtlasWidth;
			atlasItem.m_uvIn[1] = F32(scratchVewport[1] + scratchVewport[3] / tilesY * y) / scratchAtlasHeight;
			atlasItem.m_uvIn[2] = F32(scratchVewport[2] / tilesX) / scratchAtlasWidth;
			atlasItem.m_uvIn[3] = F32(scratchVewport[3] / tilesY) / scratchAtlasHeight;

			atlasItem.m_viewportOut[0] = atlasViewport[0] + atlasViewport[2] / tilesX * x;
			atlasItem.m_viewportOut[1] = atlasViewport[1] + atlasViewport[3] / tilesY * y;
			atlasItem.m_viewportOut[2] = atlasViewport[2] / tilesX;
			atlasItem.m_viewportOut[3] = atlasViewport[3] / tilesY;

			atlasItem.m_blur = blurAtlas;

			atlasResolveWorkItem.emplaceBack(atlasItem);
		}
	}
}

} // end namespace anki
