// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ClusterBinning.h>
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

ClusterBinning::ClusterBinning()
{
}

ClusterBinning::~ClusterBinning()
{
}

Error ClusterBinning::init()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/ClusterBinningSetup.ankiprogbin", m_jobSetupProg, m_jobSetupGrProg));

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/ClusterBinning.ankiprogbin", m_binningProg));

	for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/ClusterBinning.ankiprogbin", {{"OBJECT_TYPE", MutatorValue(type)}}, m_binningProg,
									 m_binningGrProgs[type]));

		ANKI_CHECK(loadShaderProgram("ShaderBinaries/ClusterBinningPackVisibles.ankiprogbin", {{"OBJECT_TYPE", MutatorValue(type)}}, m_packingProg,
									 m_packingGrProgs[type]));
	}

	return Error::kNone;
}

void ClusterBinning::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(ClusterBinning);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Allocate the clusters buffer
	{
		const U32 clusterCount = getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y() + getRenderer().getZSplitCount();
		m_runCtx.m_clustersBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(sizeof(Cluster) * clusterCount);
		m_runCtx.m_clustersHandle = rgraph.importBuffer(m_runCtx.m_clustersBuffer, BufferUsageBit::kNone);
	}

	// Setup the indirect dispatches and zero the clusters buffer
	BufferView indirectArgsBuff;
	BufferHandle indirectArgsHandle;
	{
		// Allocate memory for the indirect args
		constexpr U32 dispatchCount = U32(GpuSceneNonRenderableObjectType::kCount) * 2;
		indirectArgsBuff = GpuVisibleTransientMemoryPool::getSingleton().allocate(sizeof(DispatchIndirectArgs) * dispatchCount);
		indirectArgsHandle = rgraph.importBuffer(indirectArgsBuff, BufferUsageBit::kNone);

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

			const UVec4 consts(getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y());
			cmdb.setPushConstants(&consts, sizeof(consts));

			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				const BufferView& buff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindStorageBuffer(Register(HlslResourceType::kSrv, U32(type)), buff);
			}

			cmdb.bindStorageBuffer(ANKI_REG(u0), indirectArgsBuff);

			cmdb.dispatchCompute(1, 1, 1);

			// Now zero the clusters buffer
			cmdb.fillBuffer(m_runCtx.m_clustersBuffer, 0);
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

			PtrSize indirectArgsBuffOffset = indirectArgsBuff.getOffset();
			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				cmdb.bindShaderProgram(m_binningGrProgs[type].get());

				const BufferView& idsBuff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindStorageBuffer(ANKI_REG(t0), idsBuff);

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
					objBufferRange = GpuSceneBuffer::getSingleton().getBufferView().getRange();
				}

				cmdb.bindStorageBuffer(ANKI_REG(t1), BufferView(&GpuSceneBuffer::getSingleton().getBuffer(), objBufferOffset, objBufferRange));
				cmdb.bindStorageBuffer(ANKI_REG(u0), m_runCtx.m_clustersBuffer);

				struct ClusterBinningUniforms
				{
					Vec3 m_cameraOrigin;
					F32 m_zSplitCountOverFrustumLength;

					Vec2 m_renderingSize;
					U32 m_tileCountX;
					U32 m_tileCount;

					Vec4 m_nearPlaneWorld;

					I32 m_zSplitCountMinusOne;
					I32 m_padding0;
					I32 m_padding1;
					I32 m_padding2;

					Mat4 m_invertedViewProjMat;
				} unis;

				unis.m_cameraOrigin = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
				unis.m_zSplitCountOverFrustumLength = F32(getRenderer().getZSplitCount()) / (ctx.m_cameraFar - ctx.m_cameraNear);
				unis.m_renderingSize = Vec2(getRenderer().getInternalResolution());
				unis.m_tileCountX = getRenderer().getTileCounts().x();
				unis.m_tileCount = getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y();

				Plane nearPlane;
				extractClipPlane(ctx.m_matrices.m_viewProjection, FrustumPlaneType::kNear, nearPlane);
				unis.m_nearPlaneWorld = Vec4(nearPlane.getNormal().xyz(), nearPlane.getOffset());

				unis.m_zSplitCountMinusOne = getRenderer().getZSplitCount() - 1;

				unis.m_invertedViewProjMat = ctx.m_matrices.m_invertedViewProjectionJitter;

				cmdb.setPushConstants(&unis, sizeof(unis));

				cmdb.dispatchComputeIndirect(BufferView(indirectArgsBuff).setOffset(indirectArgsBuffOffset).setRange(sizeof(DispatchIndirectArgs)));

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
				GpuVisibleTransientMemoryPool::getSingleton().allocate(kClusteredObjectSizes[type] * kMaxVisibleClusteredObjects[type]);
			m_runCtx.m_packedObjectsHandles[type] = rgraph.importBuffer(m_runCtx.m_packedObjectsBuffers[type], BufferUsageBit::kNone);
		}

		// Create the pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Cluster object packing");
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kIndirectCompute);
		for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
		{
			rpass.newBufferDependency(m_runCtx.m_packedObjectsHandles[type], BufferUsageBit::kStorageComputeWrite);
		}

		rpass.setWork([this, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			PtrSize indirectArgsBuffOffset =
				indirectArgsBuff.getOffset() + sizeof(DispatchIndirectArgs) * U32(GpuSceneNonRenderableObjectType::kCount);
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
					objBufferRange = GpuSceneBuffer::getSingleton().getBufferView().getRange();
				}

				cmdb.bindStorageBuffer(ANKI_REG(t0), BufferView(&GpuSceneBuffer::getSingleton().getBuffer(), objBufferOffset, objBufferRange));
				cmdb.bindStorageBuffer(ANKI_REG(u0), m_runCtx.m_packedObjectsBuffers[type]);

				const BufferView& idsBuff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindStorageBuffer(ANKI_REG(t1), idsBuff);

				cmdb.dispatchComputeIndirect(BufferView(indirectArgsBuff).setOffset(indirectArgsBuffOffset).setRange(sizeof(DispatchIndirectArgs)));

				indirectArgsBuffOffset += sizeof(DispatchIndirectArgs);
			}
		});
	}
}

} // end namespace anki
