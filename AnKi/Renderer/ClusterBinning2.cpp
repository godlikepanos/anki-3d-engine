// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ClusterBinning2.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

ClusterBinning2::ClusterBinning2()
{
}

ClusterBinning2::~ClusterBinning2()
{
}

Error ClusterBinning2::init()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/ClusterBinning2Setup.ankiprogbin", m_jobSetupProg, m_jobSetupGrProg));

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/ClusterBinning2.ankiprogbin", m_binningProg));

	for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
	{
		ShaderProgramResourceVariantInitInfo inf(m_binningProg);
		inf.addMutation("OBJECT_TYPE", MutatorValue(type));
		inf.addConstant("kTileSize", getRenderer().getTileSize());
		inf.addConstant("kZSplitCount", getRenderer().getZSplitCount());
		const ShaderProgramResourceVariant* variant;
		m_binningProg->getOrCreateVariant(inf, variant);
		m_binningGrProgs[type].reset(&variant->getProgram());

		ANKI_CHECK(loadShaderProgram("ShaderBinaries/ClusterBinning2PackVisibles.ankiprogbin",
									 Array<SubMutation, 1>{{"OBJECT_TYPE", MutatorValue(type)}}, m_packingProg, m_packingGrProgs[type]));
	}

	return Error::kNone;
}

void ClusterBinning2::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Fire an async job to fill the general uniforms
	{
		m_runCtx.m_rctx = &ctx;

		m_runCtx.m_clusterUniformsBuffer = RebarTransientMemoryPool::getSingleton().allocateFrame(1, m_runCtx.m_uniformsCpu);

		CoreThreadHive::getSingleton().submitTask(
			[](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
			   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
				static_cast<ClusterBinning2*>(userData)->writeClusterUniformsInternal();
			},
			this);
	}

	// Allocate the clusters buffer
	{
		const U32 clusterCount = getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y() + getRenderer().getZSplitCount();
		m_runCtx.m_clustersBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(sizeof(Cluster) * clusterCount);
		m_runCtx.m_clustersHandle = rgraph.importBuffer(BufferUsageBit::kNone, m_runCtx.m_clustersBuffer);
	}

	// Setup the indirect dispatches and zero the clusters buffer
	BufferOffsetRange indirectArgsBuff;
	BufferHandle indirectArgsHandle;
	{
		// Allocate memory for the indirect args
		constexpr U32 dispatchCount = U32(GpuSceneNonRenderableObjectType::kCount) * 2;
		indirectArgsBuff = GpuVisibleTransientMemoryPool::getSingleton().allocate(sizeof(DispatchIndirectArgs) * dispatchCount);
		indirectArgsHandle = rgraph.importBuffer(BufferUsageBit::kNone, indirectArgsBuff);

		// Create the pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Cluster binning setup");

		for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
		{
			rpass.newBufferDependency(getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBufferHandle(type),
									  BufferUsageBit::kStorageComputeRead);
		}
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kStorageComputeWrite);
		rpass.newBufferDependency(m_runCtx.m_clustersHandle, BufferUsageBit::kTransferDestination);

		rpass.setWork([this, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_jobSetupGrProg.get());

			const UVec4 uniforms(getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y());
			cmdb.setPushConstants(&uniforms, sizeof(uniforms));

			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				const BufferOffsetRange& buff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindStorageBuffer(0, 0, buff.m_buffer, buff.m_offset, buff.m_range, U32(type));
			}

			cmdb.bindStorageBuffer(0, 1, indirectArgsBuff.m_buffer, indirectArgsBuff.m_offset, indirectArgsBuff.m_range);

			cmdb.dispatchCompute(1, 1, 1);

			// Now zero the clusters buffer
			cmdb.fillBuffer(m_runCtx.m_clustersBuffer.m_buffer, m_runCtx.m_clustersBuffer.m_offset, m_runCtx.m_clustersBuffer.m_range, 0);
		});
	}

	// Cluster binning
	{
		// Create the pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Cluster binning");

		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kIndirectCompute);
		rpass.newBufferDependency(m_runCtx.m_clustersHandle, BufferUsageBit::kStorageComputeWrite);

		rpass.setWork([this, &ctx, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			PtrSize indirectArgsBuffOffset = indirectArgsBuff.m_offset;
			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				cmdb.bindShaderProgram(m_binningGrProgs[type].get());

				const BufferOffsetRange& idsBuff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindStorageBuffer(0, 0, idsBuff.m_buffer, idsBuff.m_offset, idsBuff.m_range);

				PtrSize objBufferOffset = 0;
				PtrSize objBufferRange = 0;
				switch(type)
				{
				case GpuSceneNonRenderableObjectType::kLight:
					objBufferOffset = GpuSceneArrays::Light::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::Light::getSingleton().getBufferRange();
					break;
				case GpuSceneNonRenderableObjectType::kDecal:
					objBufferOffset = GpuSceneArrays::Decal::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::Decal::getSingleton().getBufferRange();
					break;
				case GpuSceneNonRenderableObjectType::kFogDensityVolume:
					objBufferOffset = GpuSceneArrays::FogDensityVolume::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::FogDensityVolume::getSingleton().getBufferRange();
					break;
				case GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe:
					objBufferOffset = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getBufferRange();
					break;
				case GpuSceneNonRenderableObjectType::kReflectionProbe:
					objBufferOffset = GpuSceneArrays::ReflectionProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::ReflectionProbe::getSingleton().getBufferRange();
					break;
				default:
					ANKI_ASSERT(0);
				}

				if(objBufferRange == 0)
				{
					objBufferOffset = 0;
					objBufferRange = kMaxPtrSize;
				}

				cmdb.bindStorageBuffer(0, 1, &GpuSceneBuffer::getSingleton().getBuffer(), objBufferOffset, objBufferRange);
				cmdb.bindStorageBuffer(0, 2, m_runCtx.m_clustersBuffer.m_buffer, m_runCtx.m_clustersBuffer.m_offset,
									   m_runCtx.m_clustersBuffer.m_range);

				struct ClusterBinningUniforms
				{
					Vec3 m_cameraOrigin;
					F32 m_zSplitCountOverFrustumLength;

					Vec2 m_renderingSize;
					U32 m_tileCountX;
					U32 m_tileCount;

					Vec4 m_nearPlaneWorld;

					Mat4 m_invertedViewProjMat;
				} uniforms;

				uniforms.m_cameraOrigin = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
				uniforms.m_zSplitCountOverFrustumLength = F32(getRenderer().getZSplitCount()) / (ctx.m_cameraFar - ctx.m_cameraNear);
				uniforms.m_renderingSize = Vec2(getRenderer().getInternalResolution());
				uniforms.m_tileCountX = getRenderer().getTileCounts().x();
				uniforms.m_tileCount = getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y();

				Plane nearPlane;
				extractClipPlane(ctx.m_matrices.m_viewProjection, FrustumPlaneType::kNear, nearPlane);
				uniforms.m_nearPlaneWorld = Vec4(nearPlane.getNormal().xyz(), nearPlane.getOffset());

				uniforms.m_invertedViewProjMat = ctx.m_matrices.m_invertedViewProjectionJitter;

				cmdb.setPushConstants(&uniforms, sizeof(uniforms));

				cmdb.dispatchComputeIndirect(indirectArgsBuff.m_buffer, indirectArgsBuffOffset);
				indirectArgsBuffOffset += sizeof(DispatchIndirectArgs);
			}
		});
	}

	// Object packing
	{
		// Allocations
		for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
		{
			m_runCtx.m_packedObjectsBuffers[type] =
				GpuVisibleTransientMemoryPool::getSingleton().allocate(kClusteredObjectSizes2[type] * kMaxVisibleClusteredObjects2[type]);
			m_runCtx.m_packedObjectsHandles[type] = rgraph.importBuffer(BufferUsageBit::kNone, m_runCtx.m_packedObjectsBuffers[type]);
		}

		// Create the pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Cluster object packing");
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kIndirectCompute);
		for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
		{
			rpass.newBufferDependency(m_runCtx.m_packedObjectsHandles[type], BufferUsageBit::kStorageComputeWrite);
		}

		rpass.setWork([this, &ctx, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			PtrSize indirectArgsBuffOffset = indirectArgsBuff.m_offset + sizeof(DispatchIndirectArgs) * U32(GpuSceneNonRenderableObjectType::kCount);
			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				cmdb.bindShaderProgram(m_packingGrProgs[type].get());

				PtrSize objBufferOffset = 0;
				PtrSize objBufferRange = 0;
				switch(type)
				{
				case GpuSceneNonRenderableObjectType::kLight:
					objBufferOffset = GpuSceneArrays::Light::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::Light::getSingleton().getBufferRange();
					break;
				case GpuSceneNonRenderableObjectType::kDecal:
					objBufferOffset = GpuSceneArrays::Decal::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::Decal::getSingleton().getBufferRange();
					break;
				case GpuSceneNonRenderableObjectType::kFogDensityVolume:
					objBufferOffset = GpuSceneArrays::FogDensityVolume::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::FogDensityVolume::getSingleton().getBufferRange();
					break;
				case GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe:
					objBufferOffset = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getBufferRange();
					break;
				case GpuSceneNonRenderableObjectType::kReflectionProbe:
					objBufferOffset = GpuSceneArrays::ReflectionProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::ReflectionProbe::getSingleton().getBufferRange();
					break;
				default:
					ANKI_ASSERT(0);
				}

				if(objBufferRange == 0)
				{
					objBufferOffset = 0;
					objBufferRange = kMaxPtrSize;
				}

				cmdb.bindStorageBuffer(0, 0, &GpuSceneBuffer::getSingleton().getBuffer(), objBufferOffset, objBufferRange);
				cmdb.bindStorageBuffer(0, 1, m_runCtx.m_packedObjectsBuffers[type].m_buffer, m_runCtx.m_packedObjectsBuffers[type].m_offset,
									   m_runCtx.m_packedObjectsBuffers[type].m_range);

				const BufferOffsetRange& idsBuff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindStorageBuffer(0, 2, idsBuff.m_buffer, idsBuff.m_offset, idsBuff.m_range);

				cmdb.dispatchComputeIndirect(indirectArgsBuff.m_buffer, indirectArgsBuffOffset);
				indirectArgsBuffOffset += sizeof(DispatchIndirectArgs);
			}
		});
	}
}

void ClusterBinning2::writeClusterUniformsInternal()
{
	ANKI_TRACE_SCOPED_EVENT(RWriteClusterShadingObjects);

	RenderingContext& ctx = *m_runCtx.m_rctx;
	ClusteredShadingUniforms& unis = *m_runCtx.m_uniformsCpu;

	unis.m_renderingSize = Vec2(F32(getRenderer().getInternalResolution().x()), F32(getRenderer().getInternalResolution().y()));

	unis.m_time = F32(HighRezTimer::getCurrentTime());
	unis.m_frame = getRenderer().getFrameCount() & kMaxU32;

	Plane nearPlane;
	extractClipPlane(ctx.m_matrices.m_viewProjection, FrustumPlaneType::kNear, nearPlane);
	unis.m_nearPlaneWSpace = Vec4(nearPlane.getNormal().xyz(), nearPlane.getOffset());
	unis.m_near = ctx.m_cameraNear;
	unis.m_far = ctx.m_cameraFar;
	unis.m_cameraPosition = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();

	unis.m_tileCounts = getRenderer().getTileCounts();
	unis.m_zSplitCount = getRenderer().getZSplitCount();
	unis.m_zSplitCountOverFrustumLength = F32(getRenderer().getZSplitCount()) / (ctx.m_cameraFar - ctx.m_cameraNear);
	unis.m_zSplitMagic.x() = (ctx.m_cameraNear - ctx.m_cameraFar) / (ctx.m_cameraNear * F32(getRenderer().getZSplitCount()));
	unis.m_zSplitMagic.y() = ctx.m_cameraFar / (ctx.m_cameraNear * F32(getRenderer().getZSplitCount()));
	unis.m_tileSize = getRenderer().getTileSize();
	unis.m_lightVolumeLastZSplit = getRenderer().getVolumetricLightingAccumulation().getFinalZSplit();

	unis.m_reflectionProbesMipCount = F32(getRenderer().getProbeReflections().getReflectionTextureMipmapCount());

	unis.m_matrices = ctx.m_matrices;
	unis.m_previousMatrices = ctx.m_prevMatrices;

	// Directional light
	const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
	if(dirLight)
	{
		const CameraComponent& cam = SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>();

		DirectionalLight& out = unis.m_directionalLight;

		out.m_diffuseColor = dirLight->getDiffuseColor().xyz();
		out.m_shadowCascadeCount = cam.getShadowCascadeCount();
		out.m_direction = dirLight->getDirection();
		out.m_active = 1;
		for(U32 i = 0; i < kMaxShadowCascades; ++i)
		{
			out.m_shadowCascadeDistances[i] = cam.getShadowCascadeDistance(i);
		}
		out.m_shadowLayer = (dirLight->getShadowEnabled()) ? 1 : kMaxU32; // TODO RT

		for(U cascade = 0; cascade < cam.getShadowCascadeCount(); ++cascade)
		{
			ANKI_ASSERT(m_runCtx.m_rctx->m_dirLightTextureMatrices[cascade] != Mat4::getZero());
			out.m_textureMatrices[cascade] = m_runCtx.m_rctx->m_dirLightTextureMatrices[cascade];
		}
	}
	else
	{
		unis.m_directionalLight.m_active = 0;
	}
}

} // end namespace anki
