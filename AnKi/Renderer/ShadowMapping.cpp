// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

class ShadowMapping::ViewportWorkItem
{
public:
	UVec4 m_viewport;
	Mat4 m_mvp;
	Mat3x4 m_viewMatrix;

	GpuVisibilityOutput m_visOut;
};

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
		m_tileResolution = ConfigSet::getSingleton().getRShadowMappingTileResolution();
		m_tileCountBothAxis = ConfigSet::getSingleton().getRShadowMappingTileCountPerRowOrColumn();

		ANKI_R_LOGV("Initializing shadowmapping. Atlas resolution %ux%u", m_tileResolution * m_tileCountBothAxis,
					m_tileResolution * m_tileCountBothAxis);

		// RT
		const TextureUsageBit usage = TextureUsageBit::kSampledFragment | TextureUsageBit::kSampledCompute | TextureUsageBit::kAllFramebuffer;
		TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(
			m_tileResolution * m_tileCountBothAxis, m_tileResolution * m_tileCountBothAxis, Format::kD16_Unorm, usage, "ShadowAtlas");
		ClearValue clearVal;
		clearVal.m_colorf[0] = 1.0f;
		m_atlasTex = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment, clearVal);
	}

	// Tiles
	m_tileAlloc.init(m_tileCountBothAxis, m_tileCountBothAxis, kTileAllocHierarchyCount, true);

	m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kLoad;
	m_fbDescr.bake();

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/ShadowmappingClearDepth.ankiprogbin", m_clearDepthProg));
	const ShaderProgramResourceVariant* variant;
	m_clearDepthProg->getOrCreateVariant(variant);
	m_clearDepthGrProg.reset(&variant->getProgram());

	for(U32 i = 0; i < kMaxShadowCascades; ++i)
	{
		RendererString name;
		name.sprintf("DirLight HZB #%d", i);

		const U32 cascadeResolution = (m_tileResolution * (1 << (kTileAllocHierarchyCount - 1))) >> chooseDirectionalLightShadowCascadeDetail(i);
		UVec2 size(min(cascadeResolution, 1024u));
		size /= 2;

		m_cascadeHzbRtDescrs[i] = getRenderer().create2DRenderTargetDescription(size.x(), size.y(), Format::kR8_Unorm, name);
		m_cascadeHzbRtDescrs[i].m_mipmapCount = U8(computeMaxMipmapCount2d(m_cascadeHzbRtDescrs[i].m_width, m_cascadeHzbRtDescrs[i].m_height));
		m_cascadeHzbRtDescrs[i].bake();
	}

	return Error::kNone;
}

void ShadowMapping::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RSm);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Import
	if(m_rtImportedOnce) [[likely]]
	{
		m_runCtx.m_rt = rgraph.importRenderTarget(m_atlasTex.get());
	}
	else
	{
		m_runCtx.m_rt = rgraph.importRenderTarget(m_atlasTex.get(), TextureUsageBit::kSampledFragment);
		m_rtImportedOnce = true;
	}

	// First process the lights
	processLights(ctx);

	// Build the render graph
	if(m_runCtx.m_workItems.getSize())
	{
		// Will have to create render passes

		// Compute render area
		const U32 minx = m_runCtx.m_fullViewport[0];
		const U32 miny = m_runCtx.m_fullViewport[1];
		const U32 width = m_runCtx.m_fullViewport[2] - m_runCtx.m_fullViewport[0];
		const U32 height = m_runCtx.m_fullViewport[3] - m_runCtx.m_fullViewport[1];

		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Shadowmapping");

		for(const ViewportWorkItem& work : m_runCtx.m_workItems)
		{
			pass.newBufferDependency(work.m_visOut.m_mdiDrawCountsHandle, BufferUsageBit::kIndirectDraw);
		}

		TextureSubresourceInfo subresource = TextureSubresourceInfo(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kAllFramebuffer, subresource);

		pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(),
								 BufferUsageBit::kStorageGeometryRead | BufferUsageBit::kStorageFragmentRead);

		pass.setFramebufferInfo(m_fbDescr, {}, m_runCtx.m_rt, {}, minx, miny, width, height);
		pass.setWork(1, [this](RenderPassWorkContext& rgraphCtx) {
			runShadowMapping(rgraphCtx);
		});
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

	return Mat4(sizeTextureSpace, 0.0f, 0.0f, uv.x(), 0.0f, sizeTextureSpace, 0.0f, uv.y(), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

void ShadowMapping::chooseDetail(const Vec4& cameraOrigin, const PointLightQueueElement& light, U32& tileAllocatorHierarchy,
								 U32& renderQueueElementsLod) const
{
	const F32 distFromTheCamera = (cameraOrigin - light.m_worldPosition.xyz0()).getLength() - light.m_radius;
	if(distFromTheCamera < ConfigSet::getSingleton().getLod0MaxDistance())
	{
		tileAllocatorHierarchy = kPointLightMaxTileAllocHierarchy;
		renderQueueElementsLod = 0;
	}
	else
	{
		tileAllocatorHierarchy = max(kPointLightMaxTileAllocHierarchy, 1u) - 1;
		renderQueueElementsLod = kMaxLodCount - 1;
	}
}

void ShadowMapping::chooseDetail(const Vec4& cameraOrigin, const SpotLightQueueElement& light, U32& tileAllocatorHierarchy,
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

	if(distFromTheCamera < ConfigSet::getSingleton().getLod0MaxDistance())
	{
		tileAllocatorHierarchy = kSpotLightMaxTileAllocHierarchy;
		renderQueueElementsLod = 0;
	}
	else if(distFromTheCamera < ConfigSet::getSingleton().getLod1MaxDistance())
	{
		tileAllocatorHierarchy = max(kSpotLightMaxTileAllocHierarchy, 1u) - 1;
		renderQueueElementsLod = kMaxLodCount - 1;
	}
	else
	{
		tileAllocatorHierarchy = max(kSpotLightMaxTileAllocHierarchy, 2u) - 2;
		renderQueueElementsLod = kMaxLodCount - 1;
	}
}

Bool ShadowMapping::allocateAtlasTiles(U64 lightUuid, U32 faceCount, const U64* faceTimestamps, const U32* faceIndices, const U32* drawcallsCount,
									   const U32* hierarchies, UVec4* atlasTileViewports, TileAllocatorResult* subResults)
{
	ANKI_ASSERT(lightUuid > 0);
	ANKI_ASSERT(faceCount > 0);
	ANKI_ASSERT(faceTimestamps);
	ANKI_ASSERT(faceIndices);
	ANKI_ASSERT(drawcallsCount);
	ANKI_ASSERT(hierarchies);

	for(U i = 0; i < faceCount; ++i)
	{
		Array<U32, 4> tileViewport;
		subResults[i] = m_tileAlloc.allocate(GlobalFrameIndex::getSingleton().m_value, faceTimestamps[i], lightUuid, faceIndices[i],
											 drawcallsCount[i], hierarchies[i], tileViewport);

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

template<typename TMemoryPool>
void ShadowMapping::newWorkItem(const UVec4& atlasViewport, const RenderQueue& queue, RenderGraphDescription& rgraph,
								DynamicArray<ViewportWorkItem, TMemoryPool>& workItems, RenderTargetHandle* hzbRt)
{
	ViewportWorkItem& work = *workItems.emplaceBack();

	const Array<F32, kMaxLodCount - 1> lodDistances = {ConfigSet::getSingleton().getLod0MaxDistance(),
													   ConfigSet::getSingleton().getLod1MaxDistance()};
	getRenderer().getGpuVisibility().populateRenderGraph("Shadowmapping visibility", RenderingTechnique::kDepth, queue.m_viewProjectionMatrix,
														 queue.m_cameraTransform.getTranslationPart().xyz(), lodDistances, hzbRt, rgraph,
														 work.m_visOut);

	work.m_viewport = atlasViewport;
	work.m_mvp = queue.m_viewProjectionMatrix;
	work.m_viewMatrix = queue.m_viewMatrix;
}

void ShadowMapping::processLights(RenderingContext& ctx)
{
	m_runCtx.m_fullViewport = UVec4(kMaxU32, kMaxU32, kMinU32, kMinU32);

	// Vars
	const Vec4 cameraOrigin = ctx.m_renderQueue->m_cameraTransform.getTranslationPart().xyz0();
	DynamicArray<ViewportWorkItem, MemoryPoolPtrWrapper<StackMemoryPool>> workItems(ctx.m_tempPool);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Process the directional light first.
	if(ctx.m_renderQueue->m_directionalLight.m_shadowCascadeCount > 0)
	{
		DirectionalLightQueueElement& light = ctx.m_renderQueue->m_directionalLight;

		Array<U64, kMaxShadowCascades> timestamps;
		Array<U32, kMaxShadowCascades> cascadeIndices;
		Array<U32, kMaxShadowCascades> drawcallCounts;
		Array<UVec4, kMaxShadowCascades> atlasViewports;
		Array<TileAllocatorResult, kMaxShadowCascades> subResults;
		Array<U32, kMaxShadowCascades> hierarchies;
		Array<U32, kMaxShadowCascades> renderQueueElementsLods;

		for(U32 cascade = 0; cascade < light.m_shadowCascadeCount; ++cascade)
		{
			ANKI_ASSERT(light.m_shadowRenderQueues[cascade]);

			timestamps[cascade] = GlobalFrameIndex::getSingleton().m_value; // This light is always updated
			cascadeIndices[cascade] = cascade;
			drawcallCounts[cascade] = 1; // Doesn't matter

			// Change the quality per cascade
			hierarchies[cascade] = kTileAllocHierarchyCount - 1 - chooseDirectionalLightShadowCascadeDetail(cascade);
			renderQueueElementsLods[cascade] = (cascade == 0) ? 0 : (kMaxLodCount - 1);
		}

		const Bool allocationFailed = !allocateAtlasTiles(light.m_uuid, light.m_shadowCascadeCount, &timestamps[0], &cascadeIndices[0],
														  &drawcallCounts[0], &hierarchies[0], &atlasViewports[0], &subResults[0]);

		if(!allocationFailed)
		{
			// HZB generation
			Array<RenderTargetHandle, kMaxShadowCascades> hzbRts;
			Array<UVec2, kMaxShadowCascades> hzbSizes;
			Array<Mat4, kMaxShadowCascades> dstViewProjectionMats;

			for(U cascade = 0; cascade < light.m_shadowCascadeCount; ++cascade)
			{
				hzbRts[cascade] = rgraph.newRenderTarget(m_cascadeHzbRtDescrs[cascade]);
				hzbSizes[cascade] = UVec2(m_cascadeHzbRtDescrs[cascade].m_width, m_cascadeHzbRtDescrs[cascade].m_height);
				dstViewProjectionMats[cascade] = ctx.m_renderQueue->m_directionalLight.m_shadowRenderQueues[cascade]->m_viewProjectionMatrix;
			}

			getRenderer().getHzbHelper().populateRenderGraphDirectionalLight(getRenderer().getGBuffer().getDepthRt(),
																			 getRenderer().getInternalResolution(), hzbRts, dstViewProjectionMats,
																			 hzbSizes, ctx.m_matrices.m_invertedViewProjection, rgraph);

			for(U cascade = 0; cascade < light.m_shadowCascadeCount; ++cascade)
			{
				// Update the texture matrix to point to the correct region in the atlas
				light.m_textureMatrices[cascade] = createSpotLightTextureMatrix(atlasViewports[cascade]) * light.m_textureMatrices[cascade];

				// Push work
				newWorkItem(atlasViewports[cascade], *light.m_shadowRenderQueues[cascade], rgraph, workItems, &hzbRts[cascade]);
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
		Array<U32, 6> hierarchies;

		U32 hierarchy, renderQueueElementsLod;
		chooseDetail(cameraOrigin, light, hierarchy, renderQueueElementsLod);

		for(U32 face = 0; face < 6; ++face)
		{
			ANKI_ASSERT(light.m_shadowRenderQueues[face]);

			faceIndices[face] = face;
			timestamps[face] = light.m_shadowRenderQueues[face]->m_shadowRenderablesLastUpdateTimestamp;

			drawcallCounts[face] = light.m_shadowRenderQueues[face]->m_renderables.getSize();

			hierarchies[face] = hierarchy;
		}

		const Bool allocationFailed = !allocateAtlasTiles(light.m_uuid, 6, &timestamps[0], &faceIndices[0], &drawcallCounts[0], &hierarchies[0],
														  &atlasViewports[0], &subResults[0]);

		if(!allocationFailed)
		{
			// All good, update the lights

			// Remove a few texels to avoid bilinear filtering bleeding
			F32 texelsBorder;
			if(ConfigSet::getSingleton().getRShadowMappingPcf())
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

			for(U face = 0; face < 6; ++face)
			{
				const UVec4& atlasViewport = atlasViewports[face];

				// Add a half texel to the viewport's start to avoid bilinear filtering bleeding
				light.m_shadowAtlasTileOffsets[face].x() = (F32(atlasViewport[0]) + texelsBorder) / atlasResolution;
				light.m_shadowAtlasTileOffsets[face].y() = (F32(atlasViewport[1]) + texelsBorder) / atlasResolution;

				if(subResults[face] != TileAllocatorResult::kCached)
				{
					newWorkItem(atlasViewport, *light.m_shadowRenderQueues[face], rgraph, workItems);
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

		U32 hierarchy, renderQueueElementsLod;
		chooseDetail(cameraOrigin, light, hierarchy, renderQueueElementsLod);

		const Bool allocationFailed = !allocateAtlasTiles(light.m_uuid, 1, &light.m_shadowRenderQueue->m_shadowRenderablesLastUpdateTimestamp,
														  &faceIdx, &localDrawcallCount, &hierarchy, &atlasViewport, &subResult);

		if(!allocationFailed)
		{
			// All good, update the light

			// Update the texture matrix to point to the correct region in the atlas
			light.m_textureMatrix = createSpotLightTextureMatrix(atlasViewport) * light.m_textureMatrix;

			if(subResult != TileAllocatorResult::kCached)
			{
				newWorkItem(atlasViewport, *light.m_shadowRenderQueue, rgraph, workItems);
			}
		}
		else
		{
			// Doesn't have renderables or the allocation failed, won't be a shadow caster
			light.m_shadowRenderQueue = nullptr;
		}
	}

	// Move the work to the context
	if(workItems.getSize())
	{
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
	ANKI_TRACE_SCOPED_EVENT(RSm);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

	for(ViewportWorkItem& work : m_runCtx.m_workItems)
	{
		// Set state
		cmdb.setViewport(work.m_viewport[0], work.m_viewport[1], work.m_viewport[2], work.m_viewport[3]);
		cmdb.setScissor(work.m_viewport[0], work.m_viewport[1], work.m_viewport[2], work.m_viewport[3]);

		// Clear the depth buffer
		{
			cmdb.bindShaderProgram(m_clearDepthGrProg.get());
			cmdb.setDepthCompareOperation(CompareOperation::kAlways);
			cmdb.setPolygonOffset(0.0f, 0.0f);
			cmdb.draw(PrimitiveTopology::kTriangles, 3, 1);

			// Restore state
			cmdb.setDepthCompareOperation(CompareOperation::kLess);
			cmdb.setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);
		}

		RenderableDrawerArguments args;
		args.m_renderingTechinuqe = RenderingTechnique::kDepth;
		args.m_viewMatrix = work.m_viewMatrix;
		args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
		args.m_viewProjectionMatrix = work.m_mvp;
		args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
		args.m_sampler = getRenderer().getSamplers().m_trilinearRepeatAniso.get();
		args.fillMdi(work.m_visOut);

		getRenderer().getSceneDrawer().drawMdi(args, cmdb);
	}
}

} // end namespace anki
