// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/ConfigSet.h>
#include <anki/util/ThreadHive.h>
#include <anki/util/Tracer.h>

namespace anki
{

class ShadowMapping::Scratch::WorkItem
{
public:
	Array<U32, 4> m_viewport;
	RenderQueue* m_renderQueue;
	U32 m_firstRenderableElement;
	U32 m_renderableElementCount;
	U32 m_threadPoolTaskIdx;
};

class ShadowMapping::Scratch::LightToRenderToScratchInfo
{
public:
	Array<U32, 4> m_viewport;
	RenderQueue* m_renderQueue;
	U32 m_drawcallCount;
};

class ShadowMapping::Atlas::ResolveWorkItem
{
public:
	Vec4 m_uvIn; ///< UV + size that point to the scratch buffer.
	Array<U32, 4> m_viewportOut; ///< Viewport in the atlas RT.
	Bool m_blur;
};

ShadowMapping::~ShadowMapping()
{
}

Error ShadowMapping::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing shadowmapping");

	const Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadowmapping");
	}

	ANKI_R_LOGI("\tScratch size %ux%u. atlas size %ux%u", m_scratch.m_tileCountX * m_scratch.m_tileResolution,
				m_scratch.m_tileCountY * m_scratch.m_tileResolution,
				m_atlas.m_tileCountBothAxis * m_atlas.m_tileResolution,
				m_atlas.m_tileCountBothAxis * m_atlas.m_tileResolution);

	return err;
}

Error ShadowMapping::initScratch(const ConfigSet& cfg)
{
	// Init the shadowmaps and FBs
	{
		m_scratch.m_tileCountX = cfg.getNumberU32("r_shadowMappingScratchTileCountX");
		m_scratch.m_tileCountY = cfg.getNumberU32("r_shadowMappingScratchTileCountY");
		m_scratch.m_tileResolution = cfg.getNumberU32("r_shadowMappingTileResolution");

		// RT
		m_scratch.m_rtDescr = m_r->create2DRenderTargetDescription(m_scratch.m_tileResolution * m_scratch.m_tileCountX,
																   m_scratch.m_tileResolution * m_scratch.m_tileCountY,
																   SHADOW_DEPTH_PIXEL_FORMAT, "SM scratch");
		m_scratch.m_rtDescr.bake();

		// FB
		m_scratch.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
		m_scratch.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
		m_scratch.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
		m_scratch.m_fbDescr.bake();
	}

	m_scratch.m_tileAlloc.init(getAllocator(), m_scratch.m_tileCountX, m_scratch.m_tileCountY, m_lodCount, false);

	return Error::NONE;
}

Error ShadowMapping::initAtlas(const ConfigSet& cfg)
{
	// Init RT
	{
		m_atlas.m_tileResolution = cfg.getNumberU32("r_shadowMappingTileResolution");
		m_atlas.m_tileCountBothAxis = cfg.getNumberU32("r_shadowMappingTileCountPerRowOrColumn");

		// RT
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(
			m_atlas.m_tileResolution * m_atlas.m_tileCountBothAxis,
			m_atlas.m_tileResolution * m_atlas.m_tileCountBothAxis, SHADOW_COLOR_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::IMAGE_COMPUTE_WRITE | TextureUsageBit::SAMPLED_COMPUTE,
			"SM atlas");
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		ClearValue clearVal;
		clearVal.m_colorf[0] = 1.0f;
		m_atlas.m_tex = m_r->createAndClearRenderTarget(texinit, clearVal);
	}

	// Tiles
	m_atlas.m_tileAlloc.init(getAllocator(), m_atlas.m_tileCountBothAxis, m_atlas.m_tileCountBothAxis, m_lodCount,
							 true);

	// Programs and shaders
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/ExponentialShadowmappingResolve.ankiprog",
													 m_atlas.m_resolveProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_atlas.m_resolveProg);
		variantInitInfo.addConstant("INPUT_TEXTURE_SIZE", UVec2(m_scratch.m_tileCountX * m_scratch.m_tileResolution,
																m_scratch.m_tileCountY * m_scratch.m_tileResolution));

		const ShaderProgramResourceVariant* variant;
		m_atlas.m_resolveProg->getOrCreateVariant(variantInitInfo, variant);
		m_atlas.m_resolveGrProg = variant->getProgram();
	}

	return Error::NONE;
}

Error ShadowMapping::initInternal(const ConfigSet& cfg)
{
	ANKI_CHECK(initScratch(cfg));
	ANKI_CHECK(initAtlas(cfg));

	m_lodDistances[0] = cfg.getNumberF32("r_shadowMappingLightLodDistance0");
	m_lodDistances[1] = cfg.getNumberF32("r_shadowMappingLightLodDistance1");

	return Error::NONE;
}

void ShadowMapping::runAtlas(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_atlas.m_resolveWorkItems.getSize());
	ANKI_TRACE_SCOPED_EVENT(R_SM);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_atlas.m_resolveGrProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindTexture(0, 1, m_scratch.m_rt, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindImage(0, 2, m_atlas.m_rt, {});

	for(const Atlas::ResolveWorkItem& workItem : m_atlas.m_resolveWorkItems)
	{
		ANKI_TRACE_INC_COUNTER(R_SHADOW_PASSES, 1);

		struct Uniforms
		{
			UVec4 m_viewport;
			Vec2 m_uvScale;
			Vec2 m_uvTranslation;
			U32 m_blur;
			U32 m_padding0;
			U32 m_padding1;
			U32 m_padding2;
		} unis;
		unis.m_uvScale = workItem.m_uvIn.zw();
		unis.m_uvTranslation = workItem.m_uvIn.xy();
		unis.m_viewport = UVec4(workItem.m_viewportOut[0], workItem.m_viewportOut[1], workItem.m_viewportOut[2],
								workItem.m_viewportOut[3]);
		unis.m_blur = workItem.m_blur;

		cmdb->setPushConstants(&unis, sizeof(unis));

		dispatchPPCompute(cmdb, 8, 8, workItem.m_viewportOut[2], workItem.m_viewportOut[3]);
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

		m_r->getSceneDrawer().drawRange(Pass::SM, work.m_renderQueue->m_viewMatrix,
										work.m_renderQueue->m_viewProjectionMatrix,
										Mat4::getIdentity(), // Don't care about prev matrices here
										cmdb, m_r->getSamplers().m_trilinearRepeatAniso,
										work.m_renderQueue->m_renderables.getBegin() + work.m_firstRenderableElement,
										work.m_renderQueue->m_renderables.getBegin() + work.m_firstRenderableElement
											+ work.m_renderableElementCount,
										MAX_LOD_COUNT - 1);
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
			pass.setFramebufferInfo(m_scratch.m_fbDescr, {}, m_scratch.m_rt, minx, miny, width, height);
			ANKI_ASSERT(threadCountForScratchPass
						&& threadCountForScratchPass <= m_r->getThreadHive().getThreadCount());
			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					static_cast<ShadowMapping*>(rgraphCtx.m_userData)->runShadowMapping(rgraphCtx);
				},
				this, threadCountForScratchPass);

			TextureSubresourceInfo subresource = TextureSubresourceInfo(DepthStencilAspectBit::DEPTH);
			pass.newDependency({m_scratch.m_rt, TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT, subresource});
		}

		// Atlas pass
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("SM atlas");

			m_atlas.m_rt = rgraph.importRenderTarget(m_atlas.m_tex, TextureUsageBit::SAMPLED_FRAGMENT);
			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					static_cast<ShadowMapping*>(rgraphCtx.m_userData)->runAtlas(rgraphCtx);
				},
				this, 0);

			pass.newDependency({m_scratch.m_rt, TextureUsageBit::SAMPLED_COMPUTE,
								TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});
			pass.newDependency({m_atlas.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
		}
	}
	else
	{
		// No need for shadowmapping passes, just import the atlas
		m_atlas.m_rt = rgraph.importRenderTarget(m_atlas.m_tex, TextureUsageBit::SAMPLED_FRAGMENT);
	}
}

Mat4 ShadowMapping::createSpotLightTextureMatrix(const Viewport& viewport) const
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

U32 ShadowMapping::choseLod(const Vec4& cameraOrigin, const PointLightQueueElement& light, Bool& blurAtlas) const
{
	const F32 distFromTheCamera = (cameraOrigin - light.m_worldPosition.xyz0()).getLength() - light.m_radius;
	if(distFromTheCamera < m_lodDistances[0])
	{
		ANKI_ASSERT(m_pointLightsMaxLod == 1);
		blurAtlas = true;
		return 1;
	}
	else
	{
		blurAtlas = false;
		return 0;
	}
}

U32 ShadowMapping::choseLod(const Vec4& cameraOrigin, const SpotLightQueueElement& light, Bool& blurAtlas) const
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

	U32 lod;
	if(distFromTheCamera < m_lodDistances[0])
	{
		blurAtlas = true;
		lod = 2;
	}
	else if(distFromTheCamera < m_lodDistances[1])
	{
		blurAtlas = false;
		lod = 1;
	}
	else
	{
		blurAtlas = false;
		lod = 0;
	}

	return lod;
}

TileAllocatorResult ShadowMapping::allocateTilesAndScratchTiles(U64 lightUuid, U32 faceCount, const U64* faceTimestamps,
																const U32* faceIndices, const U32* drawcallsCount,
																const U32* lods, Viewport* atlasTileViewports,
																Viewport* scratchTileViewports,
																TileAllocatorResult* subResults)
{
	ANKI_ASSERT(lightUuid > 0);
	ANKI_ASSERT(faceCount > 0);
	ANKI_ASSERT(faceTimestamps);
	ANKI_ASSERT(faceIndices);
	ANKI_ASSERT(drawcallsCount);
	ANKI_ASSERT(lods);

	TileAllocatorResult res = TileAllocatorResult::ALLOCATION_FAILED;

	// Allocate atlas tiles first. They may be cached and that will affect how many scratch tiles we'll need
	for(U i = 0; i < faceCount; ++i)
	{
		res = m_atlas.m_tileAlloc.allocate(m_r->getGlobalTimestamp(), faceTimestamps[i], lightUuid, faceIndices[i],
										   drawcallsCount[i], lods[i], atlasTileViewports[i]);

		if(res == TileAllocatorResult::ALLOCATION_FAILED)
		{
			ANKI_R_LOGW("There is not enough space in the shadow atlas for more shadow maps. "
						"Increase the r_shadowMappingTileCountPerRowOrColumn or decrease the scene's shadow casters");

			// Invalidate cache entries for what we already allocated
			for(U j = 0; j < i; ++j)
			{
				m_atlas.m_tileAlloc.invalidateCache(lightUuid, faceIndices[j]);
			}

			return res;
		}

		subResults[i] = res;

		// Fix viewport
		atlasTileViewports[i][0] *= m_atlas.m_tileResolution;
		atlasTileViewports[i][1] *= m_atlas.m_tileResolution;
		atlasTileViewports[i][2] *= m_atlas.m_tileResolution;
		atlasTileViewports[i][3] *= m_atlas.m_tileResolution;
	}

	// Allocate scratch tiles
	for(U i = 0; i < faceCount; ++i)
	{
		if(subResults[i] == TileAllocatorResult::CACHED)
		{
			continue;
		}

		ANKI_ASSERT(subResults[i] == TileAllocatorResult::ALLOCATION_SUCCEEDED);

		res = m_scratch.m_tileAlloc.allocate(m_r->getGlobalTimestamp(), faceTimestamps[i], lightUuid, faceIndices[i],
											 drawcallsCount[i], lods[i], scratchTileViewports[i]);

		if(res == TileAllocatorResult::ALLOCATION_FAILED)
		{
			ANKI_R_LOGW("Don't have enough space in the scratch shadow mapping buffer. "
						"If you see this message too often increase r_shadowMappingScratchTileCountX/Y");

			// Invalidate atlas tiles
			for(U j = 0; j < faceCount; ++j)
			{
				m_atlas.m_tileAlloc.invalidateCache(lightUuid, faceIndices[j]);
			}

			return res;
		}

		// Fix viewport
		scratchTileViewports[i][0] *= m_scratch.m_tileResolution;
		scratchTileViewports[i][1] *= m_scratch.m_tileResolution;
		scratchTileViewports[i][2] *= m_scratch.m_tileResolution;
		scratchTileViewports[i][3] *= m_scratch.m_tileResolution;

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
	DynamicArrayAuto<Scratch::LightToRenderToScratchInfo> lightsToRender(ctx.m_tempAllocator);
	U32 drawcallCount = 0;
	DynamicArrayAuto<Atlas::ResolveWorkItem> atlasWorkItems(ctx.m_tempAllocator);

	// First thing, allocate an empty tile for empty faces of point lights
	Viewport emptyTileViewport;
	{
		const TileAllocatorResult res = m_atlas.m_tileAlloc.allocate(m_r->getGlobalTimestamp(), 1, MAX_U64, 0, 1,
																	 m_pointLightsMaxLod, emptyTileViewport);

		(void)res;
#if ANKI_ENABLE_ASSERTS
		static Bool firstRun = true;
		if(firstRun)
		{
			ANKI_ASSERT(res == TileAllocatorResult::ALLOCATION_SUCCEEDED);
			firstRun = false;
		}
		else
		{
			ANKI_ASSERT(res == TileAllocatorResult::CACHED);
		}
#endif
	}

	// Process the directional light first.
	if(ctx.m_renderQueue->m_directionalLight.m_shadowCascadeCount > 0)
	{
		DirectionalLightQueueElement& light = ctx.m_renderQueue->m_directionalLight;

		Array<U64, MAX_SHADOW_CASCADES> timestamps;
		Array<U32, MAX_SHADOW_CASCADES> cascadeIndices;
		Array<U32, MAX_SHADOW_CASCADES> drawcallCounts;
		Array<Viewport, MAX_SHADOW_CASCADES> atlasViewports;
		Array<Viewport, MAX_SHADOW_CASCADES> scratchViewports;
		Array<TileAllocatorResult, MAX_SHADOW_CASCADES> subResults;
		Array<U32, MAX_SHADOW_CASCADES> lods;
		Array<Bool, MAX_SHADOW_CASCADES> blurAtlass;

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
				lods[activeCascades] = (cascade <= 1) ? (m_lodCount - 1) : (lods[0] - 1);

				++activeCascades;
			}
		}

		const Bool allocationFailed =
			activeCascades == 0
			|| allocateTilesAndScratchTiles(light.m_uuid, activeCascades, &timestamps[0], &cascadeIndices[0],
											&drawcallCounts[0], &lods[0], &atlasViewports[0], &scratchViewports[0],
											&subResults[0])
				   == TileAllocatorResult::ALLOCATION_FAILED;

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
						light.m_shadowRenderQueues[cascade], lightsToRender, atlasWorkItems, drawcallCount);

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
		Array<Viewport, 6> atlasViewports;
		Array<Viewport, 6> scratchViewports;
		Array<TileAllocatorResult, 6> subResults;
		Array<U32, 6> lods;
		U32 numOfFacesThatHaveDrawcalls = 0;

		Bool blurAtlas;
		const U32 lod = choseLod(cameraOrigin, light, blurAtlas);

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
				   == TileAllocatorResult::ALLOCATION_FAILED;

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

					const Viewport& atlasViewport = atlasViewports[numOfFacesThatHaveDrawcalls];
					const Viewport& scratchViewport = scratchViewports[numOfFacesThatHaveDrawcalls];

					// Add a half texel to the viewport's start to avoid bilinear filtering bleeding
					light.m_shadowAtlasTileOffsets[face].x() = (F32(atlasViewport[0]) + 0.5f) / atlasResolution;
					light.m_shadowAtlasTileOffsets[face].y() = (F32(atlasViewport[1]) + 0.5f) / atlasResolution;

					if(subResults[numOfFacesThatHaveDrawcalls] != TileAllocatorResult::CACHED)
					{
						newScratchAndAtlasResloveRenderWorkItems(atlasViewport, scratchViewport, blurAtlas,
																 light.m_shadowRenderQueues[face], lightsToRender,
																 atlasWorkItems, drawcallCount);
					}

					++numOfFacesThatHaveDrawcalls;
				}
				else
				{
					// Doesn't have renderables, point the face to the empty tile
					Viewport atlasViewport = emptyTileViewport;
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
		TileAllocatorResult subResult;
		Viewport atlasViewport;
		Viewport scratchViewport;
		const U32 localDrawcallCount = light.m_shadowRenderQueue->m_renderables.getSize();

		Bool blurAtlas;
		const U32 lod = choseLod(cameraOrigin, light, blurAtlas);
		const Bool allocationFailed =
			localDrawcallCount == 0
			|| allocateTilesAndScratchTiles(
				   light.m_uuid, 1, &light.m_shadowRenderQueue->m_shadowRenderablesLastUpdateTimestamp, &faceIdx,
				   &localDrawcallCount, &lod, &atlasViewport, &scratchViewport, &subResult)
				   == TileAllocatorResult::ALLOCATION_FAILED;

		if(!allocationFailed)
		{
			// All good, update the light

			// Update the texture matrix to point to the correct region in the atlas
			light.m_textureMatrix = createSpotLightTextureMatrix(atlasViewport) * light.m_textureMatrix;

			if(subResult != TileAllocatorResult::CACHED)
			{
				newScratchAndAtlasResloveRenderWorkItems(atlasViewport, scratchViewport, blurAtlas,
														 light.m_shadowRenderQueue, lightsToRender, atlasWorkItems,
														 drawcallCount);
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
		DynamicArrayAuto<Scratch::WorkItem> workItems(ctx.m_tempAllocator);
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
	const Viewport& atlasViewport, const Viewport& scratchVewport, Bool blurAtlas, RenderQueue* lightRenderQueue,
	DynamicArrayAuto<Scratch::LightToRenderToScratchInfo>& scratchWorkItem,
	DynamicArrayAuto<Atlas::ResolveWorkItem>& atlasResolveWorkItem, U32& drawcallCount) const
{
	// Scratch work item
	{
		Scratch::LightToRenderToScratchInfo toRender = {scratchVewport, lightRenderQueue,
														lightRenderQueue->m_renderables.getSize()};
		scratchWorkItem.emplaceBack(toRender);
		drawcallCount += lightRenderQueue->m_renderables.getSize();
	}

	// Atlas resolve work item
	{
		const F32 scratchAtlasWidth = F32(m_scratch.m_tileCountX * m_scratch.m_tileResolution);
		const F32 scratchAtlasHeight = F32(m_scratch.m_tileCountY * m_scratch.m_tileResolution);

		Atlas::ResolveWorkItem atlasItem;
		atlasItem.m_uvIn[0] = F32(scratchVewport[0]) / scratchAtlasWidth;
		atlasItem.m_uvIn[1] = F32(scratchVewport[1]) / scratchAtlasHeight;
		atlasItem.m_uvIn[2] = F32(scratchVewport[2]) / scratchAtlasWidth;
		atlasItem.m_uvIn[3] = F32(scratchVewport[3]) / scratchAtlasHeight;

		atlasItem.m_viewportOut = atlasViewport;
		atlasItem.m_blur = blurAtlas;

		atlasResolveWorkItem.emplaceBack(atlasItem);
	}
}

} // end namespace anki
