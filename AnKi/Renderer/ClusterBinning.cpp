// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/PackVisibleClusteredObjects.h>
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

	ANKI_CHECK(
		getExternalSubsystems().m_resourceManager->loadResource("ShaderBinaries/ClusterBinning.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("kTileSize", m_r->getTileSize());
	variantInitInfo.addConstant("kTileCountX", m_r->getTileCounts().x());
	variantInitInfo.addConstant("kTileCountY", m_r->getTileCounts().y());
	variantInitInfo.addConstant("kZSplitCount", m_r->getZSplitCount());
	variantInitInfo.addConstant("kRenderingSize",
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

	m_runCtx.m_rebarHandle = ctx.m_renderGraphDescr.importBuffer(
		m_r->getExternalSubsystems().m_rebarStagingPool->getBuffer(), BufferUsageBit::kNone,
		m_runCtx.m_clustersToken.m_offset, m_runCtx.m_clustersToken.m_range);

	const RenderQueue& rqueue = *ctx.m_renderQueue;
	if(rqueue.m_pointLights.getSize() == 0 && rqueue.m_spotLights.getSize() == 0 && rqueue.m_decals.getSize() == 0
	   && rqueue.m_reflectionProbes.getSize() == 0 && rqueue.m_fogDensityVolumes.getSize() == 0
	   && rqueue.m_giProbes.getSize() == 0) [[unlikely]]
	{
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Cluster Binning");

	pass.newBufferDependency(m_r->getPackVisibleClusteredObjects().getClusteredObjectsRenderGraphHandle(),
							 BufferUsageBit::kStorageComputeRead);

	pass.newBufferDependency(m_runCtx.m_rebarHandle, BufferUsageBit::kStorageComputeWrite);

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

		cmdb->bindShaderProgram(m_grProg);

		bindUniforms(cmdb, 0, 0, m_runCtx.m_clusteredShadingUniformsToken);
		bindStorage(cmdb, 0, 1, m_runCtx.m_clustersToken);

		for(ClusteredObjectType type : EnumIterable<ClusteredObjectType>())
		{
			m_r->getPackVisibleClusteredObjects().bindClusteredObjectBuffer(cmdb, 0, U32(type) + 2, type);
		}

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

void ClusterBinning::writeClustererBuffers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RWriteClusterShadingObjects);

	// Check limits
	RenderQueue& rqueue = *ctx.m_renderQueue;
	if(rqueue.m_pointLights.getSize() > kMaxVisiblePointLights) [[unlikely]]
	{
		ANKI_R_LOGW("Visible point lights exceed the max value by %u",
					rqueue.m_pointLights.getSize() - kMaxVisiblePointLights);
		rqueue.m_pointLights.setArray(rqueue.m_pointLights.getBegin(), kMaxVisiblePointLights);
	}

	if(rqueue.m_spotLights.getSize() > kMaxVisibleSpotLights) [[unlikely]]
	{
		ANKI_R_LOGW("Visible spot lights exceed the max value by %u",
					rqueue.m_spotLights.getSize() - kMaxVisibleSpotLights);
		rqueue.m_spotLights.setArray(rqueue.m_spotLights.getBegin(), kMaxVisibleSpotLights);
	}

	if(rqueue.m_decals.getSize() > kMaxVisibleDecals) [[unlikely]]
	{
		ANKI_R_LOGW("Visible decals exceed the max value by %u", rqueue.m_decals.getSize() - kMaxVisibleDecals);
		rqueue.m_decals.setArray(rqueue.m_decals.getBegin(), kMaxVisibleDecals);
	}

	if(rqueue.m_fogDensityVolumes.getSize() > kMaxVisibleFogDensityVolumes) [[unlikely]]
	{
		ANKI_R_LOGW("Visible fog volumes exceed the max value by %u",
					rqueue.m_fogDensityVolumes.getSize() - kMaxVisibleFogDensityVolumes);
		rqueue.m_fogDensityVolumes.setArray(rqueue.m_fogDensityVolumes.getBegin(), kMaxVisibleFogDensityVolumes);
	}

	if(rqueue.m_reflectionProbes.getSize() > kMaxVisibleReflectionProbes) [[unlikely]]
	{
		ANKI_R_LOGW("Visible reflection probes exceed the max value by %u",
					rqueue.m_reflectionProbes.getSize() - kMaxVisibleReflectionProbes);
		rqueue.m_reflectionProbes.setArray(rqueue.m_reflectionProbes.getBegin(), kMaxVisibleReflectionProbes);
	}

	if(rqueue.m_giProbes.getSize() > kMaxVisibleGlobalIlluminationProbes) [[unlikely]]
	{
		ANKI_R_LOGW("Visible GI probes exceed the max value by %u",
					rqueue.m_giProbes.getSize() - kMaxVisibleGlobalIlluminationProbes);
		rqueue.m_giProbes.setArray(rqueue.m_giProbes.getBegin(), kMaxVisibleGlobalIlluminationProbes);
	}

	// Allocate buffers
	RebarStagingGpuMemoryPool& stagingMem = *m_r->getExternalSubsystems().m_rebarStagingPool;
	stagingMem.allocateFrame(sizeof(ClusteredShadingUniforms), m_runCtx.m_clusteredShadingUniformsToken);
	stagingMem.allocateFrame(sizeof(Cluster) * m_clusterCount, m_runCtx.m_clustersToken);
}

void ClusterBinning::writeClusterBuffersAsync()
{
	m_r->getExternalSubsystems().m_threadHive->submitTask(
		[](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
		   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
			static_cast<ClusterBinning*>(userData)->writeClustererBuffersTask();
		},
		this);
}

void ClusterBinning::writeClustererBuffersTask()
{
	ANKI_TRACE_SCOPED_EVENT(RWriteClusterShadingObjects);

	RenderingContext& ctx = *m_runCtx.m_ctx;
	const RenderQueue& rqueue = *ctx.m_renderQueue;

	// General uniforms
	{
		ClusteredShadingUniforms& unis = *reinterpret_cast<ClusteredShadingUniforms*>(
			m_r->getExternalSubsystems().m_rebarStagingPool->getBufferMappedAddress()
			+ m_runCtx.m_clusteredShadingUniformsToken.m_offset);

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

		unis.m_objectCountsUpTo[ClusteredObjectType::kPointLight].x() = rqueue.m_pointLights.getSize();
		unis.m_objectCountsUpTo[ClusteredObjectType::kSpotLight].x() =
			unis.m_objectCountsUpTo[ClusteredObjectType::kSpotLight - 1].x() + rqueue.m_spotLights.getSize();
		unis.m_objectCountsUpTo[ClusteredObjectType::kDecal].x() =
			unis.m_objectCountsUpTo[ClusteredObjectType::kDecal - 1].x() + rqueue.m_decals.getSize();
		unis.m_objectCountsUpTo[ClusteredObjectType::kFogDensityVolume].x() =
			unis.m_objectCountsUpTo[ClusteredObjectType::kFogDensityVolume - 1].x()
			+ rqueue.m_fogDensityVolumes.getSize();
		unis.m_objectCountsUpTo[ClusteredObjectType::kReflectionProbe].x() =
			unis.m_objectCountsUpTo[ClusteredObjectType::kReflectionProbe - 1].x()
			+ rqueue.m_reflectionProbes.getSize();
		unis.m_objectCountsUpTo[ClusteredObjectType::kGlobalIlluminationProbe].x() =
			unis.m_objectCountsUpTo[ClusteredObjectType::kGlobalIlluminationProbe - 1].x()
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
			out.m_shadowCascadeCount = in.m_shadowCascadeCount;
			out.m_direction = in.m_direction;
			out.m_active = 1;
			for(U32 i = 0; i < kMaxShadowCascades; ++i)
			{
				out.m_shadowCascadeDistances[i] = in.m_shadowCascadesDistances[i];
			}
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
	U8* clustersAddress =
		m_r->getExternalSubsystems().m_rebarStagingPool->getBufferMappedAddress() + m_runCtx.m_clustersToken.m_offset;
	memset(clustersAddress, 0, sizeof(Cluster) * m_clusterCount);
}

} // end namespace anki
