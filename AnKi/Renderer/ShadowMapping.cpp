// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Core/App.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/Components/CameraComponent.h>

namespace anki {

static NumericCVar<U32> g_shadowMappingTileResolutionCVar(CVarSubsystem::kRenderer, "ShadowMappingTileResolution", (ANKI_PLATFORM_MOBILE) ? 128 : 256,
														  16, 2048, "Shadowmapping tile resolution");
static NumericCVar<U32> g_shadowMappingTileCountPerRowOrColumnCVar(CVarSubsystem::kRenderer, "ShadowMappingTileCountPerRowOrColumn", 32, 1, 256,
																   "Shadowmapping atlas will have this number squared number of tiles");
NumericCVar<U32> g_shadowMappingPcfCVar(CVarSubsystem::kRenderer, "ShadowMappingPcf", (ANKI_PLATFORM_MOBILE) ? 0 : 1, 0, 1,
										"Shadow PCF (CVarSubsystem::kRenderer, 0: off, 1: on)");

static StatCounter g_tilesAllocatedStatVar(StatCategory::kRenderer, "Shadow tiles (re)allocated");

class LightHash
{
public:
	union
	{
		class
		{
		public:
			U64 m_uuid : 31;
			U64 m_componentIndex : 30;
			U64 m_faceIdx : 3;
		} m_unpacked;

		U64 m_packed;
	};
};

static U64 encodeTileHash(U32 lightUuid, U32 componentIndex, U32 faceIdx)
{
	ANKI_ASSERT(faceIdx < 6);

	LightHash c;
	c.m_unpacked.m_uuid = lightUuid;
	c.m_unpacked.m_componentIndex = componentIndex;
	c.m_unpacked.m_faceIdx = faceIdx;

	return c.m_packed;
}

static LightHash decodeTileHash(U64 hash)
{
	LightHash c;
	c.m_packed = hash;
	return c;
}

class ShadowMapping::ViewportWorkItem
{
public:
	UVec4 m_viewport;
	Mat4 m_viewProjMat;
	Mat3x4 m_viewMat;

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
		m_tileResolution = g_shadowMappingTileResolutionCVar.get();
		m_tileCountBothAxis = g_shadowMappingTileCountPerRowOrColumnCVar.get();

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

	const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	return Mat4(sizeTextureSpace, 0.0f, 0.0f, uv.x(), 0.0f, sizeTextureSpace, 0.0f, uv.y(), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f)
		   * biasMat4;
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
		const U32 minx = m_runCtx.m_renderAreaMin.x();
		const U32 miny = m_runCtx.m_renderAreaMin.y();
		const U32 width = m_runCtx.m_renderAreaMax.x() - m_runCtx.m_renderAreaMin.x();
		const U32 height = m_runCtx.m_renderAreaMax.y() - m_runCtx.m_renderAreaMin.y();

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

void ShadowMapping::chooseDetail(const Vec3& cameraOrigin, const LightComponent& lightc, U32& tileAllocatorHierarchy) const
{
	if(lightc.getLightComponentType() == LightComponentType::kPoint)
	{
		const F32 distFromTheCamera = (cameraOrigin - lightc.getWorldPosition()).getLength() - lightc.getRadius();
		if(distFromTheCamera < g_lod0MaxDistanceCVar.get())
		{
			tileAllocatorHierarchy = kPointLightMaxTileAllocHierarchy;
		}
		else
		{
			tileAllocatorHierarchy = max(kPointLightMaxTileAllocHierarchy, 1u) - 1;
		}
	}
	else
	{
		ANKI_ASSERT(lightc.getLightComponentType() == LightComponentType::kSpot);

		// Get some data
		const Vec3 coneOrigin = lightc.getWorldPosition();
		const Vec3 coneDir = lightc.getDirection();
		const F32 coneAngle = lightc.getOuterAngle();

		// Compute the distance from the camera to the light cone
		const Vec3 V = cameraOrigin - coneOrigin;
		const F32 VlenSq = V.dot(V);
		const F32 V1len = V.dot(coneDir);
		const F32 distFromTheCamera = cos(coneAngle) * sqrt(VlenSq - V1len * V1len) - V1len * sin(coneAngle);

		if(distFromTheCamera < g_lod0MaxDistanceCVar.get())
		{
			tileAllocatorHierarchy = kSpotLightMaxTileAllocHierarchy;
		}
		else if(distFromTheCamera < g_lod1MaxDistanceCVar.get())
		{
			tileAllocatorHierarchy = max(kSpotLightMaxTileAllocHierarchy, 1u) - 1;
		}
		else
		{
			tileAllocatorHierarchy = max(kSpotLightMaxTileAllocHierarchy, 2u) - 2;
		}
	}
}

TileAllocatorResult2 ShadowMapping::allocateAtlasTiles(U32 lightUuid, U32 componentIndex, U32 faceCount, const U32* hierarchies,
													   UVec4* atlasTileViewports)
{
	ANKI_ASSERT(lightUuid > 0);
	ANKI_ASSERT(faceCount > 0);
	ANKI_ASSERT(hierarchies);

	TileAllocatorResult2 goodResult = TileAllocatorResult2::kAllocationSucceded | TileAllocatorResult2::kTileCached;

	for(U i = 0; i < faceCount; ++i)
	{
		TileAllocator2::ArrayOfLightUuids kickedOutLights(&getRenderer().getFrameMemoryPool());

		Array<U32, 4> tileViewport;
		const TileAllocatorResult2 result = m_tileAlloc.allocate(
			GlobalFrameIndex::getSingleton().m_value, encodeTileHash(lightUuid, componentIndex, i), hierarchies[i], tileViewport, kickedOutLights);

		for(U64 kickedLightHash : kickedOutLights)
		{
			const LightHash hash = decodeTileHash(kickedLightHash);
			const Bool found = SceneGraph::getSingleton().getComponentArrays().getLights().indexExists(hash.m_unpacked.m_componentIndex);
			if(found)
			{
				LightComponent& lightc = SceneGraph::getSingleton().getComponentArrays().getLights()[hash.m_unpacked.m_componentIndex];
				if(lightc.getUuid() == hash.m_unpacked.m_uuid)
				{
					lightc.setShadowAtlasUvViewports({});
				}
			}
		}

		if(!!(result & TileAllocatorResult2::kAllocationFailed))
		{
			ANKI_R_LOGW("There is not enough space in the shadow atlas for more shadow maps. Increase the %s or decrease the scene's shadow casters",
						g_shadowMappingTileCountPerRowOrColumnCVar.getFullName().cstr());

			// Invalidate cache entries for what we already allocated
			for(U j = 0; j < i; ++j)
			{
				m_tileAlloc.invalidateCache(encodeTileHash(lightUuid, componentIndex, j));
			}

			return TileAllocatorResult2::kAllocationFailed;
		}

		if(!(result & TileAllocatorResult2::kTileCached))
		{
			g_tilesAllocatedStatVar.increment(1);
		}

		goodResult &= result;

		// Set viewport
		const UVec4 viewport = UVec4(tileViewport) * m_tileResolution;
		atlasTileViewports[i] = viewport;

		m_runCtx.m_renderAreaMin = m_runCtx.m_renderAreaMin.min(UVec2(viewport[0], viewport[1]));
		m_runCtx.m_renderAreaMax = m_runCtx.m_renderAreaMax.max(UVec2(viewport[0] + viewport[2], viewport[1] + viewport[3]));
	}

	return goodResult;
}

void ShadowMapping::processLights(RenderingContext& ctx)
{
	m_runCtx.m_renderAreaMin = UVec2(kMaxU32, kMaxU32);
	m_runCtx.m_renderAreaMax = UVec2(kMinU32, kMinU32);

	// Vars
	const Vec3 cameraOrigin = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
	DynamicArray<ViewportWorkItem, MemoryPoolPtrWrapper<StackMemoryPool>> workItems(ctx.m_tempPool);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const CameraComponent& mainCam = SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>();

	// Process the directional light first.
	const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
	if(dirLight && dirLight->getShadowEnabled() && mainCam.getShadowCascadeCount())
	{
		const U32 cascadeCount = mainCam.getShadowCascadeCount();

		Array<U32, kMaxShadowCascades> cascadeIndices;
		Array<U32, kMaxShadowCascades> hierarchies;
		for(U32 cascade = 0; cascade < cascadeCount; ++cascade)
		{
			cascadeIndices[cascade] = cascade;

			// Change the quality per cascade
			hierarchies[cascade] = kTileAllocHierarchyCount - 1 - chooseDirectionalLightShadowCascadeDetail(cascade);
		}

		Array<UVec4, kMaxShadowCascades> atlasViewports;
		[[maybe_unused]] const TileAllocatorResult2 res = allocateAtlasTiles(kMaxU32, 0, cascadeCount, &hierarchies[0], &atlasViewports[0]);

		ANKI_ASSERT(!!(res & TileAllocatorResult2::kAllocationSucceded) && "Dir light should never fail");

		// Compute the view projection matrices
		Array<F32, kMaxShadowCascades> cascadeDistances;
		for(U32 i = 0; i < cascadeCount; ++i)
		{
			cascadeDistances[i] = mainCam.getShadowCascadeDistance(i);
		}

		Array<Mat4, kMaxShadowCascades> cascadeViewProjMats;
		Array<Mat3x4, kMaxShadowCascades> cascadeViewMats;
		dirLight->computeCascadeFrustums(mainCam.getFrustum(), {&cascadeDistances[0], cascadeCount}, {&cascadeViewProjMats[0], cascadeCount},
										 {&cascadeViewMats[0], cascadeCount});

		// HZB generation
		Array<RenderTargetHandle, kMaxShadowCascades> hzbRts;
		Array<UVec2, kMaxShadowCascades> hzbSizes;
		Array<Mat4, kMaxShadowCascades> dstViewProjectionMats;
		for(U cascade = 0; cascade < cascadeCount; ++cascade)
		{
			hzbRts[cascade] = rgraph.newRenderTarget(m_cascadeHzbRtDescrs[cascade]);
			hzbSizes[cascade] = UVec2(m_cascadeHzbRtDescrs[cascade].m_width, m_cascadeHzbRtDescrs[cascade].m_height);
			dstViewProjectionMats[cascade] = cascadeViewProjMats[cascade];
		}

		getRenderer().getHzbGenerator().populateRenderGraphDirectionalLight(
			getRenderer().getGBuffer().getDepthRt(), getRenderer().getInternalResolution(), {hzbRts.getBegin(), cascadeCount},
			{dstViewProjectionMats.getBegin(), cascadeCount}, {hzbSizes.getBegin(), cascadeCount}, ctx.m_matrices.m_invertedViewProjection, rgraph);

		// Vis testing
		for(U cascade = 0; cascade < cascadeCount; ++cascade)
		{
			ViewportWorkItem& work = *workItems.emplaceBack();
			work.m_viewProjMat = cascadeViewProjMats[cascade];
			work.m_viewMat = cascadeViewMats[cascade];
			work.m_viewport = atlasViewports[cascade];

			// Vis testing
			const Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};
			FrustumGpuVisibilityInput visIn;
			visIn.m_passesName = "Shadows visibility: Dir light";
			visIn.m_technique = RenderingTechnique::kDepth;
			visIn.m_viewProjectionMatrix = cascadeViewProjMats[cascade];
			visIn.m_lodReferencePoint = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
			visIn.m_lodDistances = lodDistances;
			visIn.m_hzbRt = &hzbRts[cascade];
			visIn.m_rgraph = &rgraph;

			getRenderer().getGpuVisibility().populateRenderGraph(visIn, work.m_visOut);

			// Update the texture matrix to point to the correct region in the atlas
			ctx.m_dirLightTextureMatrices[cascade] = createSpotLightTextureMatrix(atlasViewports[cascade]) * cascadeViewProjMats[cascade];
		}
	}

	// Process the point lights.
	WeakArray<LightComponent*> lights = getRenderer().getPrimaryNonRenderableVisibility().getInterestingVisibleComponents().m_shadowLights;
	for(LightComponent* lightc : lights)
	{
		if(lightc->getLightComponentType() != LightComponentType::kPoint || !lightc->getShadowEnabled())
		{
			continue;
		}

		// Prepare data to allocate tiles and allocate
		U32 hierarchy;
		chooseDetail(cameraOrigin, *lightc, hierarchy);
		Array<U32, 6> hierarchies;
		hierarchies.fill(hierarchy);

		Array<UVec4, 6> atlasViewports;
		const TileAllocatorResult2 result = allocateAtlasTiles(lightc->getUuid(), lightc->getArrayIndex(), 6, &hierarchies[0], &atlasViewports[0]);

		if(!!(result & TileAllocatorResult2::kAllocationSucceded))
		{
			// All good, update the light

			// Remove a few texels to avoid bilinear filtering bleeding
			F32 texelsBorder;
			if(g_shadowMappingPcfCVar.get())
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

			Array<Vec4, 6> uvViewports;
			for(U face = 0; face < 6; ++face)
			{
				// Add a half texel to the viewport's start to avoid bilinear filtering bleeding
				const Vec2 uvViewportXY = (Vec2(atlasViewports[face].xy()) + texelsBorder) / atlasResolution;

				uvViewports[face] = Vec4(uvViewportXY, Vec2(superTileSize / atlasResolution));
			}

			if(!(result & TileAllocatorResult2::kTileCached))
			{
				lightc->setShadowAtlasUvViewports(uvViewports);
			}

			// Vis testing
			const Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};
			DistanceGpuVisibilityInput visIn;
			visIn.m_passesName = "Shadows visibility: Point light";
			visIn.m_technique = RenderingTechnique::kDepth;
			visIn.m_lodReferencePoint = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;
			visIn.m_pointOfTest = lightc->getWorldPosition();
			visIn.m_testRadius = lightc->getRadius();

			GpuVisibilityOutput visOut;
			getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);

			// Add work
			for(U32 face = 0; face < 6; ++face)
			{
				Frustum frustum;
				frustum.init(FrustumType::kPerspective);
				frustum.setPerspective(kClusterObjectFrustumNearPlane, lightc->getRadius(), kPi / 2.0f, kPi / 2.0f);
				frustum.setWorldTransform(Transform(lightc->getWorldPosition().xyz0(), Frustum::getOmnidirectionalFrustumRotations()[face], 1.0f));
				frustum.update();

				ViewportWorkItem& work = *workItems.emplaceBack();
				work.m_viewProjMat = frustum.getViewProjectionMatrix();
				work.m_viewMat = frustum.getViewMatrix();
				work.m_viewport = atlasViewports[face];
				work.m_visOut = visOut;
			}
		}
		else
		{
			// Can't be a caster from now on
			lightc->setShadowAtlasUvViewports({});
		}
	}

	// Process the spot lights
	for(LightComponent* lightc : lights)
	{
		if(lightc->getLightComponentType() != LightComponentType::kSpot || !lightc->getShadowEnabled())
		{
			continue;
		}

		// Allocate tile
		U32 hierarchy;
		chooseDetail(cameraOrigin, *lightc, hierarchy);
		UVec4 atlasViewport;
		const TileAllocatorResult2 result = allocateAtlasTiles(lightc->getUuid(), lightc->getArrayIndex(), 1, &hierarchy, &atlasViewport);

		if(!!(result & TileAllocatorResult2::kAllocationSucceded))
		{
			// All good, update the light

			if(!(result & TileAllocatorResult2::kTileCached))
			{
				const F32 atlasResolution = F32(m_tileResolution * m_tileCountBothAxis);
				const Vec4 uvViewport = Vec4(atlasViewport) / atlasResolution;
				lightc->setShadowAtlasUvViewports({&uvViewport, 1});
			}

			// Vis testing
			const Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};
			FrustumGpuVisibilityInput visIn;
			visIn.m_passesName = "Shadows visibility: Spot light";
			visIn.m_technique = RenderingTechnique::kDepth;
			visIn.m_lodReferencePoint = cameraOrigin;
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;
			visIn.m_viewProjectionMatrix = lightc->getSpotLightViewProjectionMatrix();

			GpuVisibilityOutput visOut;
			getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);

			// Add work
			ViewportWorkItem& work = *workItems.emplaceBack();
			work.m_viewProjMat = lightc->getSpotLightViewProjectionMatrix();
			work.m_viewMat = lightc->getSpotLightViewMatrix();
			work.m_viewport = atlasViewport;
			work.m_visOut = visOut;
		}
		else
		{
			// Doesn't have renderables or the allocation failed, won't be a shadow caster
			lightc->setShadowAtlasUvViewports({});
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
		args.m_viewMatrix = work.m_viewMat;
		args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
		args.m_viewProjectionMatrix = work.m_viewProjMat;
		args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
		args.m_sampler = getRenderer().getSamplers().m_trilinearRepeatAniso.get();
		args.fillMdi(work.m_visOut);

		getRenderer().getSceneDrawer().drawMdi(args, cmdb);
	}
}

} // end namespace anki
