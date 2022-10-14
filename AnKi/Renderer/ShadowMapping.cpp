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

class ShadowMapping::LightToRenderTempInfo
{
public:
	UVec4 m_viewport;
	RenderQueue* m_renderQueue;
	U32 m_drawcallCount;
	U32 m_renderQueueElementsLod;
};

class ShadowMapping::ThreadWorkItem
{
public:
	UVec4 m_viewport;
	RenderQueue* m_renderQueue;
	U32 m_firstRenderableElement;
	U32 m_renderableElementCount;
	U32 m_threadPoolTaskIdx;
	U32 m_renderQueueElementsLod;
};

ShadowMapping::~ShadowMapping()
{
}

Error ShadowMapping::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadowmapping");
	}

	return err;
}

Error ShadowMapping::initInternal()
{
	// Init RT
	{
		m_tileResolution = getConfig().getRShadowMappingTileResolution();
		m_tileCountBothAxis = getConfig().getRShadowMappingTileCountPerRowOrColumn();

		ANKI_R_LOGV("Initializing shadowmapping. Atlas resolution %ux%u", m_tileResolution * m_tileCountBothAxis,
					m_tileResolution * m_tileCountBothAxis);

		// RT
		const TextureUsageBit usage =
			TextureUsageBit::kSampledFragment | TextureUsageBit::kSampledCompute | TextureUsageBit::kAllFramebuffer;
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(m_tileResolution * m_tileCountBothAxis,
																	m_tileResolution * m_tileCountBothAxis,
																	Format::kD16_Unorm, usage, "ShadowAtlas");
		ClearValue clearVal;
		clearVal.m_colorf[0] = 1.0f;
		m_atlasTex = m_r->createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment, clearVal);
	}

	// Tiles
	m_tileAlloc.init(&getMemoryPool(), m_tileCountBothAxis, m_tileCountBothAxis, kTileAllocLodCount, true);

	m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kLoad;
	m_fbDescr.bake();

	ANKI_CHECK(
		getResourceManager().loadResource("ShaderBinaries/ShadowmappingClearDepth.ankiprogbin", m_clearDepthProg));
	const ShaderProgramResourceVariant* variant;
	m_clearDepthProg->getOrCreateVariant(variant);
	m_clearDepthGrProg = variant->getProgram();

	return Error::kNone;
}

void ShadowMapping::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_SM);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Import
	if(ANKI_LIKELY(m_rtImportedOnce))
	{
		m_runCtx.m_rt = rgraph.importRenderTarget(m_atlasTex);
	}
	else
	{
		m_runCtx.m_rt = rgraph.importRenderTarget(m_atlasTex, TextureUsageBit::kSampledFragment);
		m_rtImportedOnce = true;
	}

	// First process the lights
	U32 threadCountForPass = 0;
	processLights(ctx, threadCountForPass);

	// Build the render graph
	if(m_runCtx.m_workItems.getSize())
	{
		// Will have to create render passes

		// Compute render area
		const U32 minx = m_runCtx.m_fullViewport[0];
		const U32 miny = m_runCtx.m_fullViewport[1];
		const U32 width = m_runCtx.m_fullViewport[2] - m_runCtx.m_fullViewport[0];
		const U32 height = m_runCtx.m_fullViewport[3] - m_runCtx.m_fullViewport[1];

		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("ShadowMapping");

		pass.setFramebufferInfo(m_fbDescr, {}, m_runCtx.m_rt, {}, minx, miny, width, height);
		ANKI_ASSERT(threadCountForPass && threadCountForPass <= m_r->getThreadHive().getThreadCount());
		pass.setWork(threadCountForPass, [this](RenderPassWorkContext& rgraphCtx) {
			runShadowMapping(rgraphCtx);
		});

		TextureSubresourceInfo subresource = TextureSubresourceInfo(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kAllFramebuffer, subresource);
	}
}

Mat4 ShadowMapping::createSpotLightTextureMatrix(const UVec4& viewport) const
{
	const F32 atlasSize = F32(m_tileResolution * m_tileCountBothAxis);
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

void ShadowMapping::chooseLods(const Vec4& cameraOrigin, const PointLightQueueElement& light, U32& tileBufferLod,
							   U32& renderQueueElementsLod) const
{
	const F32 distFromTheCamera = (cameraOrigin - light.m_worldPosition.xyz0()).getLength() - light.m_radius;
	if(distFromTheCamera < getConfig().getLod0MaxDistance())
	{
		ANKI_ASSERT(kPointLightMaxTileLod == 1);
		tileBufferLod = 1;
		renderQueueElementsLod = 0;
	}
	else
	{
		tileBufferLod = 0;
		renderQueueElementsLod = kMaxLodCount - 1;
	}
}

void ShadowMapping::chooseLods(const Vec4& cameraOrigin, const SpotLightQueueElement& light, U32& tileBufferLod,
							   U32& renderQueueElementsLod) const
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
		tileBufferLod = 2;
		renderQueueElementsLod = 0;
	}
	else if(distFromTheCamera < getConfig().getLod1MaxDistance())
	{
		tileBufferLod = 1;
		renderQueueElementsLod = kMaxLodCount - 1;
	}
	else
	{
		tileBufferLod = 0;
		renderQueueElementsLod = kMaxLodCount - 1;
	}
}

Bool ShadowMapping::allocateAtlasTiles(U64 lightUuid, U32 faceCount, const U64* faceTimestamps, const U32* faceIndices,
									   const U32* drawcallsCount, const U32* lods, UVec4* atlasTileViewports,
									   TileAllocatorResult* subResults)
{
	ANKI_ASSERT(lightUuid > 0);
	ANKI_ASSERT(faceCount > 0);
	ANKI_ASSERT(faceTimestamps);
	ANKI_ASSERT(faceIndices);
	ANKI_ASSERT(drawcallsCount);
	ANKI_ASSERT(lods);

	for(U i = 0; i < faceCount; ++i)
	{
		Array<U32, 4> tileViewport;
		subResults[i] = m_tileAlloc.allocate(m_r->getGlobalTimestamp(), faceTimestamps[i], lightUuid, faceIndices[i],
											 drawcallsCount[i], lods[i], tileViewport);

		if(subResults[i] == TileAllocatorResult::kAllocationFailed)
		{
			ANKI_R_LOGW("There is not enough space in the shadow atlas for more shadow maps. "
						"Increase the RShadowMappingTileCountPerRowOrColumn or decrease the scene's shadow casters");

			// Invalidate cache entries for what we already allocated
			for(U j = 0; j < i; ++j)
			{
				m_tileAlloc.invalidateCache(lightUuid, faceIndices[j]);
			}

			return false;
		}

		// Set viewport
		const UVec4 viewport = UVec4(tileViewport) * m_tileResolution;
		atlasTileViewports[i] = viewport;

		m_runCtx.m_fullViewport[0] = min(m_runCtx.m_fullViewport[0], viewport[0]);
		m_runCtx.m_fullViewport[1] = min(m_runCtx.m_fullViewport[1], viewport[1]);
		m_runCtx.m_fullViewport[2] = max(m_runCtx.m_fullViewport[2], viewport[0] + viewport[2]);
		m_runCtx.m_fullViewport[3] = max(m_runCtx.m_fullViewport[3], viewport[1] + viewport[3]);
	}

	return true;
}

void ShadowMapping::newWorkItems(const UVec4& atlasViewport, RenderQueue* lightRenderQueue, U32 renderQueueElementsLod,
								 DynamicArrayRaii<LightToRenderTempInfo>& workItems, U32& drawcallCount) const
{
	LightToRenderTempInfo toRender;
	toRender.m_renderQueue = lightRenderQueue;
	toRender.m_viewport = atlasViewport;
	toRender.m_drawcallCount = lightRenderQueue->m_renderables.getSize();
	toRender.m_renderQueueElementsLod = renderQueueElementsLod;

	workItems.emplaceBack(toRender);
	drawcallCount += toRender.m_drawcallCount;
}

void ShadowMapping::processLights(RenderingContext& ctx, U32& threadCountForPass)
{
	m_runCtx.m_fullViewport = UVec4(kMaxU32, kMaxU32, kMinU32, kMinU32);

	// Vars
	const Vec4 cameraOrigin = ctx.m_renderQueue->m_cameraTransform.getTranslationPart().xyz0();
	DynamicArrayRaii<LightToRenderTempInfo> lightsToRender(ctx.m_tempPool);
	U32 drawcallCount = 0;

	// First thing, allocate an empty tile for empty faces of point lights
	UVec4 emptyTileViewport;
	{
		Array<U32, 4> tileViewport;
		[[maybe_unused]] const TileAllocatorResult res =
			m_tileAlloc.allocate(m_r->getGlobalTimestamp(), 1, kMaxU64, 0, 1, kPointLightMaxTileLod, tileViewport);

		emptyTileViewport = UVec4(tileViewport);

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
		Array<TileAllocatorResult, kMaxShadowCascades> subResults;
		Array<U32, kMaxShadowCascades> lods;
		Array<U32, kMaxShadowCascades> renderQueueElementsLods;

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
				lods[activeCascades] = (cascade <= 1) ? (kMaxLodCount - 1) : (lods[0] - 1);
				renderQueueElementsLods[activeCascades] = (cascade == 0) ? 0 : (kMaxLodCount - 1);

				++activeCascades;
			}
		}

		const Bool allocationFailed =
			activeCascades == 0
			|| !allocateAtlasTiles(light.m_uuid, activeCascades, &timestamps[0], &cascadeIndices[0], &drawcallCounts[0],
								   &lods[0], &atlasViewports[0], &subResults[0]);

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
					newWorkItems(atlasViewports[activeCascades], light.m_shadowRenderQueues[cascade],
								 renderQueueElementsLods[activeCascades], lightsToRender, drawcallCount);

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
		Array<TileAllocatorResult, 6> subResults;
		Array<U32, 6> lods;
		U32 numOfFacesThatHaveDrawcalls = 0;

		U32 lod, renderQueueElementsLod;
		chooseLods(cameraOrigin, light, lod, renderQueueElementsLod);

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
			|| !allocateAtlasTiles(light.m_uuid, numOfFacesThatHaveDrawcalls, &timestamps[0], &faceIndices[0],
								   &drawcallCounts[0], &lods[0], &atlasViewports[0], &subResults[0]);

		if(!allocationFailed)
		{
			// All good, update the lights

			// Remove a few texels to avoid bilinear filtering bleeding
			F32 texelsBorder;
			if(getConfig().getRShadowMappingPcf())
			{
				texelsBorder = 2.0f; // 2 texels
			}
			else
			{
				texelsBorder = 0.5f; // Half texel
			}

			const F32 atlasResolution = F32(m_tileResolution * m_tileCountBothAxis);
			F32 superTileSize = F32(atlasViewports[0][2]); // Should be the same for all tiles and faces
			superTileSize -= texelsBorder * 2.0f; // Remove from both sides

			light.m_shadowAtlasTileSize = superTileSize / atlasResolution;

			numOfFacesThatHaveDrawcalls = 0;
			for(U face = 0; face < 6; ++face)
			{
				if(light.m_shadowRenderQueues[face]->m_renderables.getSize())
				{
					// Has drawcalls, asigned it to a tile

					const UVec4& atlasViewport = atlasViewports[numOfFacesThatHaveDrawcalls];

					// Add a half texel to the viewport's start to avoid bilinear filtering bleeding
					light.m_shadowAtlasTileOffsets[face].x() = (F32(atlasViewport[0]) + texelsBorder) / atlasResolution;
					light.m_shadowAtlasTileOffsets[face].y() = (F32(atlasViewport[1]) + texelsBorder) / atlasResolution;

					if(subResults[numOfFacesThatHaveDrawcalls] != TileAllocatorResult::kCached)
					{
						newWorkItems(atlasViewport, light.m_shadowRenderQueues[face], renderQueueElementsLod,
									 lightsToRender, drawcallCount);
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

					light.m_shadowAtlasTileOffsets[face].x() = (F32(atlasViewport[0]) + texelsBorder) / atlasResolution;
					light.m_shadowAtlasTileOffsets[face].y() = (F32(atlasViewport[1]) + texelsBorder) / atlasResolution;
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

		U32 lod, renderQueueElementsLod;
		chooseLods(cameraOrigin, light, lod, renderQueueElementsLod);

		const Bool allocationFailed =
			localDrawcallCount == 0
			|| !allocateAtlasTiles(light.m_uuid, 1, &light.m_shadowRenderQueue->m_shadowRenderablesLastUpdateTimestamp,
								   &faceIdx, &localDrawcallCount, &lod, &atlasViewport, &subResult);

		if(!allocationFailed)
		{
			// All good, update the light

			// Update the texture matrix to point to the correct region in the atlas
			light.m_textureMatrix = createSpotLightTextureMatrix(atlasViewport) * light.m_textureMatrix;

			if(subResult != TileAllocatorResult::kCached)
			{
				newWorkItems(atlasViewport, light.m_shadowRenderQueue, renderQueueElementsLod, lightsToRender,
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
		DynamicArrayRaii<ThreadWorkItem> workItems(ctx.m_tempPool);
		LightToRenderTempInfo* lightToRender = lightsToRender.getBegin();
		U32 lightToRenderDrawcallCount = lightToRender->m_drawcallCount;
		const LightToRenderTempInfo* lightToRenderEnd = lightsToRender.getEnd();

		const U32 threadCount = computeNumberOfSecondLevelCommandBuffers(drawcallCount);
		threadCountForPass = threadCount;
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

				ThreadWorkItem workItem;
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
		workItems.moveAndReset(m_runCtx.m_workItems);
	}
	else
	{
		m_runCtx.m_workItems = {};
	}
}

void ShadowMapping::runShadowMapping(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_runCtx.m_workItems.getSize());
	ANKI_TRACE_SCOPED_EVENT(R_SM);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const U threadIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;

	cmdb->setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

	for(ThreadWorkItem& work : m_runCtx.m_workItems)
	{
		if(work.m_threadPoolTaskIdx != threadIdx)
		{
			continue;
		}

		// Set state
		cmdb->setViewport(work.m_viewport[0], work.m_viewport[1], work.m_viewport[2], work.m_viewport[3]);
		cmdb->setScissor(work.m_viewport[0], work.m_viewport[1], work.m_viewport[2], work.m_viewport[3]);

		// The 1st drawcall will clear the depth buffer
		if(work.m_firstRenderableElement == 0)
		{
			cmdb->bindShaderProgram(m_clearDepthGrProg);
			cmdb->setDepthCompareOperation(CompareOperation::kAlways);
			cmdb->setPolygonOffset(0.0f, 0.0f);
			cmdb->drawArrays(PrimitiveTopology::kTriangles, 3, 1);

			// Restore state
			cmdb->setDepthCompareOperation(CompareOperation::kLess);
			cmdb->setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);
		}

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

} // end namespace anki
