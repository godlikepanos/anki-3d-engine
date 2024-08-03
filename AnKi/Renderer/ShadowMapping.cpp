// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Renderer/Utils/Drawer.h>
#include <AnKi/Renderer/Utils/HzbGenerator.h>
#include <AnKi/Core/App.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/RenderStateBucket.h>

namespace anki {

static NumericCVar<U32> g_shadowMappingTileResolutionCVar(CVarSubsystem::kRenderer, "ShadowMappingTileResolution", (ANKI_PLATFORM_MOBILE) ? 128 : 256,
														  16, 2048, "Shadowmapping tile resolution");
static NumericCVar<U32> g_shadowMappingTileCountPerRowOrColumnCVar(CVarSubsystem::kRenderer, "ShadowMappingTileCountPerRowOrColumn", 32, 1, 256,
																   "Shadowmapping atlas will have this number squared number of tiles");
NumericCVar<U32> g_shadowMappingPcfCVar(CVarSubsystem::kRenderer, "ShadowMappingPcf", (ANKI_PLATFORM_MOBILE) ? 0 : 1, 0, 1,
										"Shadow PCF (CVarSubsystem::kRenderer, 0: off, 1: on)");

static StatCounter g_tilesAllocatedStatVar(StatCategory::kRenderer, "Shadow tiles (re)allocated", StatFlag::kMainThreadUpdates);

class LightHash
{
public:
	class Unpacked
	{
	public:
		U64 m_uuid : 31;
		U64 m_componentIndex : 30;
		U64 m_faceIdx : 3;
	};

	union
	{
		Unpacked m_unpacked;
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

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/ShadowMappingClearDepth.ankiprogbin", m_clearDepthProg, m_clearDepthGrProg));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/ShadowMappingVetVisibility.ankiprogbin", m_vetVisibilityProg, m_vetVisibilityGrProg));

	for(U32 i = 0; i < kMaxShadowCascades; ++i)
	{
		RendererString name;
		name.sprintf("DirLight HZB #%d", i);

		const U32 cascadeResolution = (m_tileResolution * (1 << (kTileAllocHierarchyCount - 1))) >> chooseDirectionalLightShadowCascadeDetail(i);
		UVec2 size(min(cascadeResolution, 1024u));
		size /= 2;

		m_cascadeHzbRtDescrs[i] = getRenderer().create2DRenderTargetDescription(size.x(), size.y(), Format::kR32_Sfloat, name);
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
	ANKI_TRACE_SCOPED_EVENT(ShadowMapping);

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

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
}

void ShadowMapping::chooseDetail(const Vec3& cameraOrigin, const LightComponent& lightc, Vec2 lodDistances, U32& tileAllocatorHierarchy) const
{
	if(lightc.getLightComponentType() == LightComponentType::kPoint)
	{
		const F32 distFromTheCamera = (cameraOrigin - lightc.getWorldPosition()).getLength() - lightc.getRadius();
		if(distFromTheCamera < lodDistances[0])
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

		if(distFromTheCamera < lodDistances[0])
		{
			tileAllocatorHierarchy = kSpotLightMaxTileAllocHierarchy;
		}
		else if(distFromTheCamera < lodDistances[1])
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

	for(U32 i = 0; i < faceCount; ++i)
	{
		TileAllocator::ArrayOfLightUuids kickedOutLights(&getRenderer().getFrameMemoryPool());

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
			for(U32 j = 0; j < i; ++j)
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
	}

	return goodResult;
}

void ShadowMapping::processLights(RenderingContext& ctx)
{
	// First allocate tiles for the dir light and then build passes for points and spot lights. Then passes for the dir light. The dir light has many
	// passes and it will push the other types of lights further into the future. So do those first.

	// Vars
	const Vec3 cameraOrigin = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;
	const CameraComponent& mainCam = SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>();

	// Allocate tiles for the dir light first but don't build any passes
	const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
	if(dirLight && (!dirLight->getShadowEnabled() || !g_shadowCascadeCountCVar.get()))
	{
		dirLight = nullptr; // Skip dir light
	}

	Array<UVec4, kMaxShadowCascades> dirLightAtlasViewports;
	if(dirLight)
	{
		const U32 cascadeCount = g_shadowCascadeCountCVar.get();

		Array<U32, kMaxShadowCascades> hierarchies;
		for(U32 cascade = 0; cascade < cascadeCount; ++cascade)
		{
			// Change the quality per cascade
			hierarchies[cascade] = kTileAllocHierarchyCount - 1 - chooseDirectionalLightShadowCascadeDetail(cascade);
		}

		[[maybe_unused]] const TileAllocatorResult2 res = allocateAtlasTiles(kMaxU32, 0, cascadeCount, &hierarchies[0], &dirLightAtlasViewports[0]);
		ANKI_ASSERT(!!(res & TileAllocatorResult2::kAllocationSucceded) && "Dir light should never fail");
	}

	// Process the point lights first
	U32 lightIdx = 0;
	WeakArray<LightComponent*> lights = getRenderer().getPrimaryNonRenderableVisibility().getInterestingVisibleComponents().m_shadowLights;
	for(LightComponent* lightc : lights)
	{
		if(lightc->getLightComponentType() != LightComponentType::kPoint || !lightc->getShadowEnabled())
		{
			continue;
		}

		// Prepare data to allocate tiles and allocate
		U32 hierarchy;
		chooseDetail(cameraOrigin, *lightc, {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()}, hierarchy);
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
			visIn.m_passesName = generateTempPassName("Shadows point light lightIdx:%u", lightIdx);
			visIn.m_technique = RenderingTechnique::kDepth;
			visIn.m_lodReferencePoint = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;
			visIn.m_pointOfTest = lightc->getWorldPosition();
			visIn.m_testRadius = lightc->getRadius();
			visIn.m_hashVisibles = true;

			GpuVisibilityOutput visOut;
			getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);

			// Vet visibility
			const Bool renderAllways = !(result & TileAllocatorResult2::kTileCached);
			BufferView clearTileIndirectArgs;
			if(!renderAllways)
			{
				clearTileIndirectArgs =
					createVetVisibilityPass(generateTempPassName("Shadows: Vet point light lightIdx:%u", lightIdx), *lightc, visOut, rgraph);
			}

			// Draw
			Array<ShadowSubpassInfo, 6> subpasses;
			for(U32 face = 0; face < 6; ++face)
			{
				Frustum frustum;
				frustum.init(FrustumType::kPerspective);
				frustum.setPerspective(kClusterObjectFrustumNearPlane, lightc->getRadius(), kPi / 2.0f, kPi / 2.0f);
				frustum.setWorldTransform(
					Transform(lightc->getWorldPosition().xyz0(), Frustum::getOmnidirectionalFrustumRotations()[face], Vec4(1.0f, 1.0f, 1.0f, 0.0f)));
				frustum.update();

				subpasses[face].m_clearTileIndirectArgs = clearTileIndirectArgs;
				subpasses[face].m_viewMat = frustum.getViewMatrix();
				subpasses[face].m_viewport = atlasViewports[face];
				subpasses[face].m_viewProjMat = frustum.getViewProjectionMatrix();
			}

			createDrawShadowsPass(subpasses, visOut, generateTempPassName("Shadows: Point light lightIdx:%u", lightIdx), rgraph);
		}
		else
		{
			// Can't be a caster from now on
			lightc->setShadowAtlasUvViewports({});
		}

		++lightIdx;
	}

	// Process the spot lights 2nd
	lightIdx = 0;
	for(LightComponent* lightc : lights)
	{
		if(lightc->getLightComponentType() != LightComponentType::kSpot || !lightc->getShadowEnabled())
		{
			continue;
		}

		// Allocate tile
		U32 hierarchy;
		chooseDetail(cameraOrigin, *lightc, {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()}, hierarchy);
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
			visIn.m_passesName = generateTempPassName("Shadows spot light lightIdx:%u", lightIdx);
			visIn.m_technique = RenderingTechnique::kDepth;
			visIn.m_lodReferencePoint = cameraOrigin;
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;
			visIn.m_viewProjectionMatrix = lightc->getSpotLightViewProjectionMatrix();
			visIn.m_hashVisibles = true;
			visIn.m_viewportSize = atlasViewport.zw();

			GpuVisibilityOutput visOut;
			getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);

			// Vet visibility
			const Bool renderAllways = !(result & TileAllocatorResult2::kTileCached);
			BufferView clearTileIndirectArgs;
			if(!renderAllways)
			{
				clearTileIndirectArgs =
					createVetVisibilityPass(generateTempPassName("Shadows: Vet spot light lightIdx:%u", lightIdx), *lightc, visOut, rgraph);
			}

			// Add draw pass
			createDrawShadowsPass(atlasViewport, lightc->getSpotLightViewProjectionMatrix(), lightc->getSpotLightViewMatrix(), visOut,
								  clearTileIndirectArgs, {}, generateTempPassName("Shadows: Spot light lightIdx:%u", lightIdx), rgraph);
		}
		else
		{
			// Doesn't have renderables or the allocation failed, won't be a shadow caster
			lightc->setShadowAtlasUvViewports({});
		}
	}

	// Process the directional light last
	if(dirLight)
	{
		const U32 cascadeCount = g_shadowCascadeCountCVar.get();

		// Compute the view projection matrices
		Array<F32, kMaxShadowCascades> cascadeDistances;
		static_assert(kMaxShadowCascades == 4);
		cascadeDistances[0] = g_shadowCascade0DistanceCVar.get();
		cascadeDistances[1] = g_shadowCascade1DistanceCVar.get();
		cascadeDistances[2] = g_shadowCascade2DistanceCVar.get();
		cascadeDistances[3] = g_shadowCascade3DistanceCVar.get();

		Array<Mat4, kMaxShadowCascades> cascadeViewProjMats;
		Array<Mat3x4, kMaxShadowCascades> cascadeViewMats;
		Array<Mat4, kMaxShadowCascades> cascadeProjMats;
		dirLight->computeCascadeFrustums(mainCam.getFrustum(), {&cascadeDistances[0], cascadeCount}, {&cascadeProjMats[0], cascadeCount},
										 {&cascadeViewMats[0], cascadeCount});
		for(U cascade = 0; cascade < cascadeCount; ++cascade)
		{
			cascadeViewProjMats[cascade] = cascadeProjMats[cascade] * Mat4(cascadeViewMats[cascade], Vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		// HZB generation
		HzbDirectionalLightInput hzbGenIn;
		hzbGenIn.m_cascadeCount = cascadeCount;
		hzbGenIn.m_depthBufferRt = getRenderer().getGBuffer().getDepthRt();
		hzbGenIn.m_depthBufferRtSize = getRenderer().getInternalResolution();
		hzbGenIn.m_cameraProjectionMatrix = ctx.m_matrices.m_projection;
		hzbGenIn.m_cameraInverseViewProjectionMatrix = ctx.m_matrices.m_invertedViewProjection;
		for(U cascade = 0; cascade < cascadeCount; ++cascade)
		{
			hzbGenIn.m_cascades[cascade].m_hzbRt = rgraph.newRenderTarget(m_cascadeHzbRtDescrs[cascade]);
			hzbGenIn.m_cascades[cascade].m_hzbRtSize = UVec2(m_cascadeHzbRtDescrs[cascade].m_width, m_cascadeHzbRtDescrs[cascade].m_height);
			hzbGenIn.m_cascades[cascade].m_viewMatrix = cascadeViewMats[cascade];
			hzbGenIn.m_cascades[cascade].m_projectionMatrix = cascadeProjMats[cascade];
			hzbGenIn.m_cascades[cascade].m_cascadeMaxDistance = cascadeDistances[cascade];
		}

		getRenderer().getHzbGenerator().populateRenderGraphDirectionalLight(hzbGenIn, rgraph);

		// Create passes per cascade
		for(U32 cascade = 0; cascade < cascadeCount; ++cascade)
		{
			// Vis testing
			const Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};
			FrustumGpuVisibilityInput visIn;
			visIn.m_passesName = generateTempPassName("Shadows: Dir light cascade lightIdx:%u cascade:%u", lightIdx, cascade);
			visIn.m_technique = RenderingTechnique::kDepth;
			visIn.m_viewProjectionMatrix = cascadeViewProjMats[cascade];
			visIn.m_lodReferencePoint = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
			visIn.m_lodDistances = lodDistances;
			visIn.m_hzbRt = &hzbGenIn.m_cascades[cascade].m_hzbRt;
			visIn.m_rgraph = &rgraph;
			visIn.m_viewportSize = dirLightAtlasViewports[cascade].zw();

			GpuVisibilityOutput visOut;
			getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);

			// Draw
			createDrawShadowsPass(dirLightAtlasViewports[cascade], cascadeViewProjMats[cascade], cascadeViewMats[cascade], visOut, {},
								  hzbGenIn.m_cascades[cascade].m_hzbRt,
								  generateTempPassName("Shadows: Dir light lightIdx:%u cascade:%u", lightIdx, cascade), rgraph);

			// Update the texture matrix to point to the correct region in the atlas
			ctx.m_dirLightTextureMatrices[cascade] = createSpotLightTextureMatrix(dirLightAtlasViewports[cascade]) * cascadeViewProjMats[cascade];
		}
	}
}

BufferView ShadowMapping::createVetVisibilityPass(CString passName, const LightComponent& lightc, const GpuVisibilityOutput& visOut,
												  RenderGraphBuilder& rgraph) const
{
	BufferView clearTileIndirectArgs;

	clearTileIndirectArgs = GpuVisibleTransientMemoryPool::getSingleton().allocate(sizeof(DrawIndirectArgs));

	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(passName);

	// The shader doesn't actually write to the handle but have it as a write dependency for the drawer to correctly wait for this pass
	pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kStorageComputeWrite);

	pass.setWork([this, &lightc, hashBuff = visOut.m_visiblesHashBuffer, mdiBuff = visOut.m_legacy.m_mdiDrawCountsBuffer, clearTileIndirectArgs,
				  dispatchMeshIndirectArgs = visOut.m_mesh.m_dispatchMeshIndirectArgsBuffer,
				  drawIndirectArgs = visOut.m_mesh.m_drawIndirectArgs](RenderPassWorkContext& rpass) {
		CommandBuffer& cmdb = *rpass.m_commandBuffer;

		cmdb.bindShaderProgram(m_vetVisibilityGrProg.get());

		const UVec4 lightIndex(lightc.getGpuSceneLightAllocation().getIndex());
		cmdb.setPushConstants(&lightIndex, sizeof(lightIndex));

		cmdb.bindStorageBuffer(ANKI_REG(t0), hashBuff);
		cmdb.bindStorageBuffer(ANKI_REG(u0), mdiBuff.isValid() ? mdiBuff : BufferView(&getRenderer().getDummyBuffer()).setRange(sizeof(U32)));
		cmdb.bindStorageBuffer(ANKI_REG(u1), GpuSceneArrays::Light::getSingleton().getBufferView());
		cmdb.bindStorageBuffer(ANKI_REG(u2), GpuSceneArrays::LightVisibleRenderablesHash::getSingleton().getBufferView());
		cmdb.bindStorageBuffer(ANKI_REG(u3), clearTileIndirectArgs);
		cmdb.bindStorageBuffer(ANKI_REG(u4), dispatchMeshIndirectArgs.isValid()
												 ? dispatchMeshIndirectArgs
												 : BufferView(&getRenderer().getDummyBuffer()).setRange(sizeof(DispatchIndirectArgs)));
		cmdb.bindStorageBuffer(ANKI_REG(u5), drawIndirectArgs.isValid()
												 ? drawIndirectArgs
												 : BufferView(&getRenderer().getDummyBuffer()).setRange(sizeof(DrawIndirectArgs)));

		ANKI_ASSERT(RenderStateBucketContainer::getSingleton().getBucketCount(RenderingTechnique::kDepth) <= 64 && "TODO");
		cmdb.dispatchCompute(1, 1, 1);
	});

	return clearTileIndirectArgs;
}

void ShadowMapping::createDrawShadowsPass(const UVec4& viewport, const Mat4& viewProjMat, const Mat3x4& viewMat, const GpuVisibilityOutput& visOut,
										  const BufferView& clearTileIndirectArgs, const RenderTargetHandle hzbRt, CString passName,
										  RenderGraphBuilder& rgraph)
{
	ShadowSubpassInfo spass;
	spass.m_clearTileIndirectArgs = clearTileIndirectArgs;
	spass.m_hzbRt = hzbRt;
	spass.m_viewMat = viewMat;
	spass.m_viewport = viewport;
	spass.m_viewProjMat = viewProjMat;

	createDrawShadowsPass({&spass, 1}, visOut, passName, rgraph);
}

void ShadowMapping::createDrawShadowsPass(ConstWeakArray<ShadowSubpassInfo> subpasses_, const GpuVisibilityOutput& visOut, CString passName,
										  RenderGraphBuilder& rgraph)
{
	WeakArray<ShadowSubpassInfo> subpasses;
	newArray<ShadowSubpassInfo>(getRenderer().getFrameMemoryPool(), subpasses_.getSize(), subpasses);
	memcpy(subpasses.getBegin(), subpasses_.getBegin(), subpasses.getSizeInBytes());

	// Compute the whole viewport
	UVec4 viewport;
	if(subpasses.getSize() == 1)
	{
		viewport = subpasses[0].m_viewport;
	}
	else
	{
		viewport = UVec4(kMaxU32, kMaxU32, 0, 0);
		for(const ShadowSubpassInfo& s : subpasses)
		{
			viewport.x() = min(viewport.x(), s.m_viewport.x());
			viewport.y() = min(viewport.y(), s.m_viewport.y());
			viewport.z() = max(viewport.z(), s.m_viewport.x() + s.m_viewport.z());
			viewport.w() = max(viewport.w(), s.m_viewport.y() + s.m_viewport.w());
		}
		viewport.z() -= viewport.x();
		viewport.w() -= viewport.y();
	}

	// Create the pass
	GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(passName);

	const Bool loadFb = !(subpasses.getSize() == 1 && subpasses[0].m_clearTileIndirectArgs.isValid());

	GraphicsRenderPassTargetDesc smRti(m_runCtx.m_rt);
	smRti.m_loadOperation = (loadFb) ? RenderTargetLoadOperation::kLoad : RenderTargetLoadOperation::kClear;
	smRti.m_clearValue.m_depthStencil.m_depth = 1.0f;
	smRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
	pass.setRenderpassInfo({}, &smRti, viewport[0], viewport[1], viewport[2], viewport[3]);

	pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kIndirectDraw);
	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite);

	pass.setWork([this, visOut, subpasses, loadFb](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(ShadowMapping);

		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		for(U32 i = 0; i < subpasses.getSize(); ++i)
		{
			const ShadowSubpassInfo& spass = subpasses[i];

			cmdb.setViewport(spass.m_viewport[0], spass.m_viewport[1], spass.m_viewport[2], spass.m_viewport[3]);

			if(loadFb)
			{
				cmdb.bindShaderProgram(m_clearDepthGrProg.get());
				cmdb.setDepthCompareOperation(CompareOperation::kAlways);

				if(spass.m_clearTileIndirectArgs.isValid())
				{

					cmdb.drawIndirect(PrimitiveTopology::kTriangles, spass.m_clearTileIndirectArgs);
				}
				else
				{
					cmdb.draw(PrimitiveTopology::kTriangles, 3);
				}

				cmdb.setDepthCompareOperation(CompareOperation::kLess);
			}

			// Set state
			cmdb.setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

			RenderableDrawerArguments args;
			args.m_renderingTechinuqe = RenderingTechnique::kDepth;
			args.m_viewMatrix = spass.m_viewMat;
			args.m_cameraTransform = spass.m_viewMat.getInverseTransformation();
			args.m_viewProjectionMatrix = spass.m_viewProjMat;
			args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
			args.m_sampler = getRenderer().getSamplers().m_trilinearRepeat.get();
			args.m_viewport = UVec4(spass.m_viewport[0], spass.m_viewport[1], spass.m_viewport[2], spass.m_viewport[3]);
			args.fill(visOut);

			if(spass.m_hzbRt.isValid())
			{
				args.m_hzbTexture = rgraphCtx.createTextureView(spass.m_hzbRt, TextureSubresourceDesc::all());
			}

			getRenderer().getRenderableDrawer().drawMdi(args, cmdb);
		}
	});
}

} // end namespace anki
