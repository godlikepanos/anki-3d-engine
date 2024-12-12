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
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/ClusterBinning.ankiprogbin", {{"OBJECT_TYPE", 0}}, m_prog, m_jobSetupGrProg, "Setup"));

	for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/ClusterBinning.ankiprogbin", {{"OBJECT_TYPE", MutatorValue(type)}}, m_prog,
									 m_binningGrProgs[type], "Binning"));

		ANKI_CHECK(loadShaderProgram("ShaderBinaries/ClusterBinning.ankiprogbin", {{"OBJECT_TYPE", MutatorValue(type)}}, m_prog,
									 m_packingGrProgs[type], "PackVisibles"));
	}

	return Error::kNone;
}

void ClusterBinning::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(ClusterBinning);

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// Allocate the clusters buffer
	{
		const U32 clusterCount = getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y() + getRenderer().getZSplitCount();
		m_runCtx.m_clustersBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<Cluster>(clusterCount);
		m_runCtx.m_dep = rgraph.importBuffer(m_runCtx.m_clustersBuffer, BufferUsageBit::kNone);
	}

	// Setup the indirect dispatches and zero the clusters buffer
	BufferView indirectArgsBuff;
	{
		// Allocate memory for the indirect args
		constexpr U32 dispatchCount = U32(GpuSceneNonRenderableObjectType::kCount) * 2;
		indirectArgsBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<DispatchIndirectArgs>(dispatchCount);

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Cluster binning setup");

		for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
		{
			rpass.newBufferDependency(getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBufferHandle(type),
									  BufferUsageBit::kSrvCompute);
		}
		rpass.newBufferDependency(m_runCtx.m_dep, BufferUsageBit::kCopyDestination | BufferUsageBit::kUavCompute);

		rpass.setWork([this, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_jobSetupGrProg.get());

			const UVec4 consts(getRenderer().getTileCounts().x() * getRenderer().getTileCounts().y());
			cmdb.setFastConstants(&consts, sizeof(consts));

			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				const BufferView& buff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindSrv(U32(type), 0, buff);
			}

			cmdb.bindUav(0, 0, indirectArgsBuff);

			cmdb.dispatchCompute(1, 1, 1);

			// Now zero the clusters buffer
			cmdb.fillBuffer(m_runCtx.m_clustersBuffer, 0);
		});
	}

	// Cluster binning
	{
		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Cluster binning");

		rpass.newBufferDependency(m_runCtx.m_dep, BufferUsageBit::kUavCompute | BufferUsageBit::kIndirectCompute);

		rpass.setWork([this, &ctx, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			PtrSize indirectArgsBuffOffset = indirectArgsBuff.getOffset();
			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				cmdb.bindShaderProgram(m_binningGrProgs[type].get());

				const BufferView& idsBuff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindSrv(0, 0, idsBuff);

				PtrSize objBufferOffset = 0;
				PtrSize objBufferRange = 0;
				U32 elementSize = 0;
				switch(type)
				{
				case GpuSceneNonRenderableObjectType::kLight:
					objBufferOffset = GpuSceneArrays::Light::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::Light::getSingleton().getBufferRange();
					elementSize = GpuSceneArrays::Light::getElementSize();
					break;
				case GpuSceneNonRenderableObjectType::kDecal:
					objBufferOffset = GpuSceneArrays::Decal::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::Decal::getSingleton().getBufferRange();
					elementSize = GpuSceneArrays::Decal::getElementSize();
					break;
				case GpuSceneNonRenderableObjectType::kFogDensityVolume:
					objBufferOffset = GpuSceneArrays::FogDensityVolume::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::FogDensityVolume::getSingleton().getBufferRange();
					elementSize = GpuSceneArrays::FogDensityVolume::getElementSize();
					break;
				case GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe:
					objBufferOffset = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getBufferRange();
					elementSize = GpuSceneArrays::GlobalIlluminationProbe::getElementSize();
					break;
				case GpuSceneNonRenderableObjectType::kReflectionProbe:
					objBufferOffset = GpuSceneArrays::ReflectionProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::ReflectionProbe::getSingleton().getBufferRange();
					elementSize = GpuSceneArrays::ReflectionProbe::getElementSize();
					break;
				default:
					ANKI_ASSERT(0);
				}

				if(objBufferRange == 0)
				{
					objBufferOffset = 0;
					objBufferRange = getAlignedRoundDown(elementSize, GpuSceneBuffer::getSingleton().getBufferView().getRange());
				}

				cmdb.bindSrv(1, 0, BufferView(&GpuSceneBuffer::getSingleton().getBuffer(), objBufferOffset, objBufferRange));
				cmdb.bindUav(0, 0, m_runCtx.m_clustersBuffer);

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

				cmdb.setFastConstants(&consts, sizeof(consts));

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
			m_runCtx.m_packedObjectsBuffers[type] = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer(
				kMaxVisibleClusteredObjects[type], kClusteredObjectSizes[type]);
		}

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Cluster object packing");
		rpass.newBufferDependency(m_runCtx.m_dep, BufferUsageBit::kIndirectCompute | BufferUsageBit::kUavCompute);

		rpass.setWork([this, indirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			PtrSize indirectArgsBuffOffset =
				indirectArgsBuff.getOffset() + sizeof(DispatchIndirectArgs) * U32(GpuSceneNonRenderableObjectType::kCount);
			for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
			{
				cmdb.bindShaderProgram(m_packingGrProgs[type].get());

				PtrSize objBufferOffset = 0;
				PtrSize objBufferRange = 0;
				U32 objSize = 0;
				switch(type)
				{
				case GpuSceneNonRenderableObjectType::kLight:
					objBufferOffset = GpuSceneArrays::Light::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::Light::getSingleton().getBufferRange();
					objSize = GpuSceneArrays::Light::getSingleton().getElementSize();
					break;
				case GpuSceneNonRenderableObjectType::kDecal:
					objBufferOffset = GpuSceneArrays::Decal::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::Decal::getSingleton().getBufferRange();
					objSize = GpuSceneArrays::Decal::getSingleton().getElementSize();
					break;
				case GpuSceneNonRenderableObjectType::kFogDensityVolume:
					objBufferOffset = GpuSceneArrays::FogDensityVolume::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::FogDensityVolume::getSingleton().getBufferRange();
					objSize = GpuSceneArrays::FogDensityVolume::getSingleton().getElementSize();
					break;
				case GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe:
					objBufferOffset = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getBufferRange();
					objSize = GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementSize();
					break;
				case GpuSceneNonRenderableObjectType::kReflectionProbe:
					objBufferOffset = GpuSceneArrays::ReflectionProbe::getSingleton().getGpuSceneOffsetOfArrayBase();
					objBufferRange = GpuSceneArrays::ReflectionProbe::getSingleton().getBufferRange();
					objSize = GpuSceneArrays::ReflectionProbe::getSingleton().getElementSize();
					break;
				default:
					ANKI_ASSERT(0);
				}

				if(objBufferRange == 0)
				{
					objBufferOffset = 0;
					objBufferRange = getAlignedRoundDown(objSize, GpuSceneBuffer::getSingleton().getBufferView().getRange());
				}

				cmdb.bindSrv(0, 0, BufferView(&GpuSceneBuffer::getSingleton().getBuffer(), objBufferOffset, objBufferRange));
				cmdb.bindUav(0, 0, m_runCtx.m_packedObjectsBuffers[type]);

				const BufferView& idsBuff = getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type);
				cmdb.bindSrv(1, 0, idsBuff);

				cmdb.dispatchComputeIndirect(BufferView(indirectArgsBuff).setOffset(indirectArgsBuffOffset).setRange(sizeof(DispatchIndirectArgs)));

				indirectArgsBuffOffset += sizeof(DispatchIndirectArgs);
			}
		});
	}
}

} // end namespace anki
