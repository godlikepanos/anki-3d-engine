// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Collision/Plane.h>
#include <AnKi/Collision/Functions.h>

namespace anki {

ClusterBinning::ClusterBinning(Renderer* r)
	: RendererObject(r)
{
}

ClusterBinning::~ClusterBinning()
{
}

Error ClusterBinning::init()
{
	ANKI_R_LOGV("Initializing clusterer binning");

	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/ClusterBinning.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("TILE_SIZE", m_r->getTileSize());
	variantInitInfo.addConstant("TILE_COUNT_X", m_r->getTileCounts().x());
	variantInitInfo.addConstant("TILE_COUNT_Y", m_r->getTileCounts().y());
	variantInitInfo.addConstant("Z_SPLIT_COUNT", m_r->getZSplitCount());
	variantInitInfo.addConstant("RENDERING_SIZE",
								UVec2(m_r->getInternalResolution().x(), m_r->getInternalResolution().y()));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	m_tileCount = m_r->getTileCounts().x() * m_r->getTileCounts().y();
	m_clusterCount = m_tileCount + m_r->getZSplitCount();

	return Error::kNone;
}

void ClusterBinning::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	writeClustererBuffers(ctx);

	ctx.m_clusteredShading.m_clustersBufferHandle = ctx.m_renderGraphDescr.importBuffer(
		ctx.m_clusteredShading.m_clustersToken.m_buffer, BufferUsageBit::NONE,
		ctx.m_clusteredShading.m_clustersToken.m_offset, ctx.m_clusteredShading.m_clustersToken.m_range);

	const RenderQueue& rqueue = *ctx.m_renderQueue;
	if(ANKI_LIKELY(rqueue.m_pointLights.getSize() || rqueue.m_spotLights.getSize() || rqueue.m_decals.getSize()
				   || rqueue.m_reflectionProbes.getSize() || rqueue.m_fogDensityVolumes.getSize()
				   || rqueue.m_giProbes.getSize()))
	{
		RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Cluster Binning");

		pass.newDependency(
			RenderPassDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::STORAGE_COMPUTE_WRITE));

		pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

			const ClusteredShadingContext& tokens = ctx.m_clusteredShading;

			cmdb->bindShaderProgram(m_grProg);
			bindUniforms(cmdb, 0, 0, tokens.m_clusteredShadingUniformsToken);
			bindStorage(cmdb, 0, 1, tokens.m_clustersToken);
			bindUniforms(cmdb, 0, 2, tokens.m_pointLightsToken);
			bindUniforms(cmdb, 0, 3, tokens.m_spotLightsToken);
			bindUniforms(cmdb, 0, 4, tokens.m_reflectionProbesToken);
			bindUniforms(cmdb, 0, 5, tokens.m_globalIlluminationProbesToken);
			bindUniforms(cmdb, 0, 6, tokens.m_fogDensityVolumesToken);
			bindUniforms(cmdb, 0, 7, tokens.m_decalsToken);

			const U32 sampleCount = 4;
			const U32 sizex = m_tileCount * sampleCount;
			const RenderQueue& rqueue = *ctx.m_renderQueue;
			U32 clusterObjectCounts = rqueue.m_pointLights.getSize();
			clusterObjectCounts += rqueue.m_spotLights.getSize();
			clusterObjectCounts += rqueue.m_reflectionProbes.getSize();
			clusterObjectCounts += rqueue.m_giProbes.getSize();
			clusterObjectCounts += rqueue.m_fogDensityVolumes.getSize();
			clusterObjectCounts += rqueue.m_decals.getSize();
			cmdb->dispatchCompute((sizex + 64 - 1) / 64, clusterObjectCounts, 1);
		});
	}
}

void ClusterBinning::writeClustererBuffers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_WRITE_CLUSTER_SHADING_OBJECTS);

	// Check limits
	RenderQueue& rqueue = *ctx.m_renderQueue;
	if(ANKI_UNLIKELY(rqueue.m_pointLights.getSize() > MAX_VISIBLE_POINT_LIGHTS))
	{
		ANKI_R_LOGW("Visible point lights exceed the max value by %u",
					rqueue.m_pointLights.getSize() - MAX_VISIBLE_POINT_LIGHTS);
		rqueue.m_pointLights.setArray(rqueue.m_pointLights.getBegin(), MAX_VISIBLE_POINT_LIGHTS);
	}

	if(ANKI_UNLIKELY(rqueue.m_spotLights.getSize() > MAX_VISIBLE_SPOT_LIGHTS))
	{
		ANKI_R_LOGW("Visible spot lights exceed the max value by %u",
					rqueue.m_spotLights.getSize() - MAX_VISIBLE_SPOT_LIGHTS);
		rqueue.m_spotLights.setArray(rqueue.m_spotLights.getBegin(), MAX_VISIBLE_SPOT_LIGHTS);
	}

	if(ANKI_UNLIKELY(rqueue.m_decals.getSize() > MAX_VISIBLE_DECALS))
	{
		ANKI_R_LOGW("Visible decals exceed the max value by %u", rqueue.m_decals.getSize() - MAX_VISIBLE_DECALS);
		rqueue.m_decals.setArray(rqueue.m_decals.getBegin(), MAX_VISIBLE_DECALS);
	}

	if(ANKI_UNLIKELY(rqueue.m_fogDensityVolumes.getSize() > MAX_VISIBLE_FOG_DENSITY_VOLUMES))
	{
		ANKI_R_LOGW("Visible fog volumes exceed the max value by %u",
					rqueue.m_fogDensityVolumes.getSize() - MAX_VISIBLE_FOG_DENSITY_VOLUMES);
		rqueue.m_fogDensityVolumes.setArray(rqueue.m_fogDensityVolumes.getBegin(), MAX_VISIBLE_FOG_DENSITY_VOLUMES);
	}

	if(ANKI_UNLIKELY(rqueue.m_reflectionProbes.getSize() > MAX_VISIBLE_REFLECTION_PROBES))
	{
		ANKI_R_LOGW("Visible reflection probes exceed the max value by %u",
					rqueue.m_reflectionProbes.getSize() - MAX_VISIBLE_REFLECTION_PROBES);
		rqueue.m_reflectionProbes.setArray(rqueue.m_reflectionProbes.getBegin(), MAX_VISIBLE_REFLECTION_PROBES);
	}

	if(ANKI_UNLIKELY(rqueue.m_giProbes.getSize() > MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES))
	{
		ANKI_R_LOGW("Visible GI probes exceed the max value by %u",
					rqueue.m_giProbes.getSize() - MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES);
		rqueue.m_giProbes.setArray(rqueue.m_giProbes.getBegin(), MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES);
	}

	// Allocate buffers
	ClusteredShadingContext& cs = ctx.m_clusteredShading;
	StagingGpuMemoryPool& stagingMem = m_r->getStagingGpuMemory();

	cs.m_clusteredShadingUniformsAddress = stagingMem.allocateFrame(
		sizeof(ClusteredShadingUniforms), StagingGpuMemoryType::UNIFORM, cs.m_clusteredShadingUniformsToken);

	if(rqueue.m_pointLights.getSize())
	{
		cs.m_pointLightsAddress = stagingMem.allocateFrame(rqueue.m_pointLights.getSize() * sizeof(PointLight),
														   StagingGpuMemoryType::UNIFORM, cs.m_pointLightsToken);
	}
	else
	{
		cs.m_pointLightsToken.markUnused();
	}

	if(rqueue.m_spotLights.getSize())
	{
		cs.m_spotLightsAddress = stagingMem.allocateFrame(rqueue.m_spotLights.getSize() * sizeof(SpotLight),
														  StagingGpuMemoryType::UNIFORM, cs.m_spotLightsToken);
	}
	else
	{
		cs.m_spotLightsToken.markUnused();
	}

	if(rqueue.m_reflectionProbes.getSize())
	{
		cs.m_reflectionProbesAddress =
			stagingMem.allocateFrame(rqueue.m_reflectionProbes.getSize() * sizeof(ReflectionProbe),
									 StagingGpuMemoryType::UNIFORM, cs.m_reflectionProbesToken);
	}
	else
	{
		cs.m_reflectionProbesToken.markUnused();
	}

	if(rqueue.m_decals.getSize())
	{
		cs.m_decalsAddress = stagingMem.allocateFrame(rqueue.m_decals.getSize() * sizeof(Decal),
													  StagingGpuMemoryType::UNIFORM, cs.m_decalsToken);
	}
	else
	{
		cs.m_decalsToken.markUnused();
	}

	if(rqueue.m_fogDensityVolumes.getSize())
	{
		cs.m_fogDensityVolumesAddress =
			stagingMem.allocateFrame(rqueue.m_fogDensityVolumes.getSize() * sizeof(FogDensityVolume),
									 StagingGpuMemoryType::UNIFORM, cs.m_fogDensityVolumesToken);
	}
	else
	{
		cs.m_fogDensityVolumesToken.markUnused();
	}

	if(rqueue.m_giProbes.getSize())
	{
		cs.m_globalIlluminationProbesAddress =
			stagingMem.allocateFrame(rqueue.m_giProbes.getSize() * sizeof(GlobalIlluminationProbe),
									 StagingGpuMemoryType::UNIFORM, cs.m_globalIlluminationProbesToken);
	}
	else
	{
		cs.m_globalIlluminationProbesToken.markUnused();
	}

	cs.m_clustersAddress =
		stagingMem.allocateFrame(sizeof(Cluster) * m_clusterCount, StagingGpuMemoryType::STORAGE, cs.m_clustersToken);
}

void ClusterBinning::writeClusterBuffersAsync()
{
	m_r->getThreadHive().submitTask(
		[](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
		   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
			static_cast<ClusterBinning*>(userData)->writeClustererBuffersTask();
		},
		this);
}

void ClusterBinning::writeClustererBuffersTask()
{
	ANKI_TRACE_SCOPED_EVENT(R_WRITE_CLUSTER_SHADING_OBJECTS);

	RenderingContext& ctx = *m_runCtx.m_ctx;
	ClusteredShadingContext& cs = ctx.m_clusteredShading;
	const RenderQueue& rqueue = *ctx.m_renderQueue;

	// Point lights
	if(rqueue.m_pointLights.getSize())
	{
		PointLight* lights = static_cast<PointLight*>(cs.m_pointLightsAddress);

		for(U32 i = 0; i < rqueue.m_pointLights.getSize(); ++i)
		{
			PointLight& out = lights[i];
			const PointLightQueueElement& in = rqueue.m_pointLights[i];

			out.m_position = in.m_worldPosition;
			out.m_diffuseColor = in.m_diffuseColor;
			out.m_radius = in.m_radius;
			out.m_squareRadiusOverOne = 1.0f / (in.m_radius * in.m_radius);

			if(in.m_shadowRenderQueues[0] == nullptr)
			{
				out.m_shadowAtlasTileScale = -1.0f;
				out.m_shadowLayer = kMaxU32;
			}
			else
			{
				out.m_shadowLayer = in.m_shadowLayer;
				out.m_shadowAtlasTileScale = in.m_shadowAtlasTileSize;
				for(U32 f = 0; f < 6; ++f)
				{
					out.m_shadowAtlasTileOffsets[f] = Vec4(in.m_shadowAtlasTileOffsets[f], 0.0f, 0.0f);
				}
			}
		}
	}

	// Spot lights
	if(rqueue.m_spotLights.getSize())
	{
		SpotLight* lights = static_cast<SpotLight*>(cs.m_spotLightsAddress);

		for(U32 i = 0; i < rqueue.m_spotLights.getSize(); ++i)
		{
			const SpotLightQueueElement& in = rqueue.m_spotLights[i];
			SpotLight& out = lights[i];

			out.m_position = in.m_worldTransform.getTranslationPart().xyz();
			for(U32 j = 0; j < 4; ++j)
			{
				out.m_edgePoints[j] = in.m_edgePoints[j].xyz0();
			}
			out.m_diffuseColor = in.m_diffuseColor;
			out.m_radius = in.m_distance;
			out.m_squareRadiusOverOne = 1.0f / (in.m_distance * in.m_distance);
			out.m_shadowLayer = (in.hasShadow()) ? in.m_shadowLayer : kMaxU32;
			out.m_direction = -in.m_worldTransform.getRotationPart().getZAxis();
			out.m_outerCos = cos(in.m_outerAngle / 2.0f);
			out.m_innerCos = cos(in.m_innerAngle / 2.0f);

			if(in.hasShadow())
			{
				// bias * proj_l * view_l
				out.m_textureMatrix = in.m_textureMatrix;
			}
			else
			{
				out.m_textureMatrix = Mat4::getIdentity();
			}
		}
	}

	// Reflection probes
	if(rqueue.m_reflectionProbes.getSize())
	{
		ReflectionProbe* probes = static_cast<ReflectionProbe*>(cs.m_reflectionProbesAddress);

		for(U32 i = 0; i < rqueue.m_reflectionProbes.getSize(); ++i)
		{
			const ReflectionProbeQueueElement& in = rqueue.m_reflectionProbes[i];
			ReflectionProbe& out = probes[i];

			out.m_position = in.m_worldPosition;
			out.m_cubemapIndex = F32(in.m_textureArrayIndex);
			out.m_aabbMin = in.m_aabbMin;
			out.m_aabbMax = in.m_aabbMax;
		}
	}

	// Decals
	if(rqueue.m_decals.getSize())
	{
		Decal* decals = static_cast<Decal*>(cs.m_decalsAddress);

		TextureView* diffuseAtlas = nullptr;
		TextureView* specularRoughnessAtlas = nullptr;
		for(U32 i = 0; i < rqueue.m_decals.getSize(); ++i)
		{
			const DecalQueueElement& in = rqueue.m_decals[i];
			Decal& out = decals[i];

			if((diffuseAtlas != nullptr && diffuseAtlas != in.m_diffuseAtlas)
			   || (specularRoughnessAtlas != nullptr && specularRoughnessAtlas != in.m_specularRoughnessAtlas))
			{
				ANKI_R_LOGF("All decals should have the same tex atlas");
			}

			diffuseAtlas = in.m_diffuseAtlas;
			specularRoughnessAtlas = in.m_specularRoughnessAtlas;

			// Diff
			Vec4 uv = in.m_diffuseAtlasUv;
			out.m_diffuseUv = Vec4(uv.x(), uv.y(), uv.z() - uv.x(), uv.w() - uv.y());
			out.m_blendFactors[0] = in.m_diffuseAtlasBlendFactor;

			// Other
			uv = in.m_specularRoughnessAtlasUv;
			out.m_normRoughnessUv = Vec4(uv.x(), uv.y(), uv.z() - uv.x(), uv.w() - uv.y());
			out.m_blendFactors[1] = in.m_specularRoughnessAtlasBlendFactor;

			// bias * proj_l * view
			out.m_textureMatrix = in.m_textureMatrix;

			// Transform
			const Mat4 trf(in.m_obbCenter.xyz1(), in.m_obbRotation, 1.0f);
			out.m_invertedTransform = trf.getInverse();
			out.m_obbExtend = in.m_obbExtend;
		}

		ANKI_ASSERT(diffuseAtlas || specularRoughnessAtlas);
		ctx.m_clusteredShading.m_diffuseDecalTextureView.reset(diffuseAtlas);
		ctx.m_clusteredShading.m_specularRoughnessDecalTextureView.reset(specularRoughnessAtlas);
	}

	// Fog volumes
	if(rqueue.m_fogDensityVolumes.getSize())
	{
		FogDensityVolume* volumes = static_cast<FogDensityVolume*>(cs.m_fogDensityVolumesAddress);

		for(U32 i = 0; i < rqueue.m_fogDensityVolumes.getSize(); ++i)
		{
			const FogDensityQueueElement& in = rqueue.m_fogDensityVolumes[i];
			FogDensityVolume& out = volumes[i];

			out.m_density = in.m_density;
			if(in.m_isBox)
			{
				out.m_isBox = 1;
				out.m_aabbMinOrSphereCenter = in.m_aabbMin;
				out.m_aabbMaxOrSphereRadiusSquared = in.m_aabbMax;
			}
			else
			{
				out.m_isBox = 0;
				out.m_aabbMinOrSphereCenter = in.m_sphereCenter;
				out.m_aabbMaxOrSphereRadiusSquared = Vec3(in.m_sphereRadius * in.m_sphereRadius);
			}
		}
	}

	// GI
	if(rqueue.m_giProbes.getSize())
	{
		GlobalIlluminationProbe* probes = static_cast<GlobalIlluminationProbe*>(cs.m_globalIlluminationProbesAddress);

		for(U32 i = 0; i < rqueue.m_giProbes.getSize(); ++i)
		{
			const GlobalIlluminationProbeQueueElement& in = rqueue.m_giProbes[i];
			GlobalIlluminationProbe& out = probes[i];

			out.m_aabbMin = in.m_aabbMin;
			out.m_aabbMax = in.m_aabbMax;
			out.m_textureIndex = U32(&in - &rqueue.m_giProbes.getFront());
			out.m_halfTexelSizeU = 1.0f / F32(F32(in.m_cellCounts.x()) * 6.0f) / 2.0f;
			out.m_fadeDistance = in.m_fadeDistance;
		}
	}

	// General uniforms
	{
		ClusteredShadingUniforms& unis = *static_cast<ClusteredShadingUniforms*>(cs.m_clusteredShadingUniformsAddress);

		unis.m_renderingSize = Vec2(F32(m_r->getInternalResolution().x()), F32(m_r->getInternalResolution().y()));

		unis.m_time = F32(HighRezTimer::getCurrentTime());
		unis.m_frame = m_r->getFrameCount() & kMaxU32;

		Plane nearPlane;
		extractClipPlane(rqueue.m_viewProjectionMatrix, FrustumPlaneType::kNear, nearPlane);
		unis.m_nearPlaneWSpace = Vec4(nearPlane.getNormal().xyz(), nearPlane.getOffset());
		unis.m_near = rqueue.m_cameraNear;
		unis.m_far = rqueue.m_cameraFar;
		unis.m_cameraPosition = rqueue.m_cameraTransform.getTranslationPart().xyz();

		unis.m_tileCounts = m_r->getTileCounts();
		unis.m_zSplitCount = m_r->getZSplitCount();
		unis.m_zSplitCountOverFrustumLength = F32(m_r->getZSplitCount()) / (rqueue.m_cameraFar - rqueue.m_cameraNear);
		unis.m_zSplitMagic.x() =
			(rqueue.m_cameraNear - rqueue.m_cameraFar) / (rqueue.m_cameraNear * F32(m_r->getZSplitCount()));
		unis.m_zSplitMagic.y() = rqueue.m_cameraFar / (rqueue.m_cameraNear * F32(m_r->getZSplitCount()));
		unis.m_tileSize = m_r->getTileSize();
		unis.m_lightVolumeLastZSplit = m_r->getVolumetricLightingAccumulation().getFinalZSplit();

		unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_POINT_LIGHT].x() = rqueue.m_pointLights.getSize();
		unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_SPOT_LIGHT].x() =
			unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_SPOT_LIGHT - 1].x() + rqueue.m_spotLights.getSize();
		unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_DECAL].x() =
			unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_DECAL - 1].x() + rqueue.m_decals.getSize();
		unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_FOG_DENSITY_VOLUME].x() =
			unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_FOG_DENSITY_VOLUME - 1].x()
			+ rqueue.m_fogDensityVolumes.getSize();
		unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_REFLECTION_PROBE].x() =
			unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_REFLECTION_PROBE - 1].x() + rqueue.m_reflectionProbes.getSize();
		unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_GLOBAL_ILLUMINATION_PROBE].x() =
			unis.m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_GLOBAL_ILLUMINATION_PROBE - 1].x()
			+ rqueue.m_giProbes.getSize();

		unis.m_reflectionProbesMipCount = F32(m_r->getProbeReflections().getReflectionTextureMipmapCount());

		unis.m_matrices = ctx.m_matrices;
		unis.m_previousMatrices = ctx.m_prevMatrices;

		// Directional light
		if(rqueue.m_directionalLight.m_uuid != 0)
		{
			DirectionalLight& out = unis.m_directionalLight;
			const DirectionalLightQueueElement& in = rqueue.m_directionalLight;

			out.m_diffuseColor = in.m_diffuseColor;
			out.m_cascadeCount = in.m_shadowCascadeCount;
			out.m_direction = in.m_direction;
			out.m_active = 1;
			out.m_effectiveShadowDistance = in.m_effectiveShadowDistance;
			out.m_shadowCascadesDistancePower = in.m_shadowCascadesDistancePower;
			out.m_shadowLayer = (in.hasShadow()) ? in.m_shadowLayer : kMaxU32;

			for(U cascade = 0; cascade < in.m_shadowCascadeCount; ++cascade)
			{
				out.m_textureMatrices[cascade] = in.m_textureMatrices[cascade];
			}
		}
		else
		{
			unis.m_directionalLight.m_active = 0;
		}
	}

	// Zero the memory because atomics will happen
	memset(cs.m_clustersAddress, 0, sizeof(Cluster) * m_clusterCount);
}

} // end namespace anki
