// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
									  BufferUsageBit::kUavComputeRead);
		}
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kUavComputeWrite);
		rpass.newBufferDependency(m_runCtx.m_clustersHandle, BufferUsageBit::kTransferDestination);

		rpass.setWork([this, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_jobSetupGrProg.get());

			const UVec4 consts(getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y());
			cmdb.setPushConstants(&consts, sizeof(consts));

			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				const BufferOffsetRange& buff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindUavBuffer(0, 0, buff.m_buffer, buff.m_offset, buff.m_range, U32(type));
			}

			cmdb.bindUavBuffer(0, 1, indirectArgsBuff.m_buffer, indirectArgsBuff.m_offset, indirectArgsBuff.m_range);

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
		rpass.newBufferDependency(m_runCtx.m_clustersHandle, BufferUsageBit::kUavComputeWrite);

		rpass.setWork([this, &ctx, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			PtrSize indirectArgsBuffOffset = indirectArgsBuff.m_offset;
			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				cmdb.bindShaderProgram(m_binningGrProgs[type].get());

				const BufferOffsetRange& idsBuff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindUavBuffer(0, 0, idsBuff.m_buffer, idsBuff.m_offset, idsBuff.m_range);

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

				cmdb.bindUavBuffer(0, 1, &GpuSceneBuffer::getSingleton().getBuffer(), objBufferOffset, objBufferRange);
				cmdb.bindUavBuffer(0, 2, m_runCtx.m_clustersBuffer.m_buffer, m_runCtx.m_clustersBuffer.m_offset, m_runCtx.m_clustersBuffer.m_range);

				struct ClusterBinningConstants
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
				} consts;

				consts.m_cameraOrigin = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
				consts.m_zSplitCountOverFrustumLength = F32(getRenderer().getZSplitCount()) / (ctx.m_cameraFar - ctx.m_cameraNear);
				consts.m_renderingSize = Vec2(getRenderer().getInternalResolution());
				consts.m_tileCountX = getRenderer().getTileCounts().x();
				consts.m_tileCount = getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y();

				Plane nearPlane;
				extractClipPlane(ctx.m_matrices.m_viewProjection, FrustumPlaneType::kNear, nearPlane);
				consts.m_nearPlaneWorld = Vec4(nearPlane.getNormal().xyz(), nearPlane.getOffset());

				consts.m_zSplitCountMinusOne = getRenderer().getZSplitCount() - 1;

				consts.m_invertedViewProjMat = ctx.m_matrices.m_invertedViewProjectionJitter;

				cmdb.setPushConstants(&consts, sizeof(consts));

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
				GpuVisibleTransientMemoryPool::getSingleton().allocate(kClusteredObjectSizes[type] * kMaxVisibleClusteredObjects[type]);
			m_runCtx.m_packedObjectsHandles[type] = rgraph.importBuffer(BufferUsageBit::kNone, m_runCtx.m_packedObjectsBuffers[type]);
		}

		// Create the pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Cluster object packing");
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kIndirectCompute);
		for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
		{
			rpass.newBufferDependency(m_runCtx.m_packedObjectsHandles[type], BufferUsageBit::kUavComputeWrite);
		}

		rpass.setWork([this, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
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

				cmdb.bindUavBuffer(0, 0, &GpuSceneBuffer::getSingleton().getBuffer(), objBufferOffset, objBufferRange);
				cmdb.bindUavBuffer(0, 1, m_runCtx.m_packedObjectsBuffers[type].m_buffer, m_runCtx.m_packedObjectsBuffers[type].m_offset,
								   m_runCtx.m_packedObjectsBuffers[type].m_range);

				const BufferOffsetRange& idsBuff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindUavBuffer(0, 2, idsBuff.m_buffer, idsBuff.m_offset, idsBuff.m_range);

				cmdb.dispatchComputeIndirect(indirectArgsBuff.m_buffer, indirectArgsBuffOffset);
				indirectArgsBuffOffset += sizeof(DispatchIndirectArgs);
			}
		});
	}
}

} // end namespace anki
