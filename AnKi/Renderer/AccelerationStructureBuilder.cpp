// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/App.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>

namespace anki {

static NumericCVar<F32>
	g_rayTracingExtendedFrustumDistanceCVar(CVarSubsystem::kRenderer, "RayTracingExtendedFrustumDistance", 100.0f, 10.0f, 10000.0f,
											"Every object that its distance from the camera is bellow that value will take part in ray tracing");

void AccelerationStructureBuilder::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(ASBuilder);

	// Do visibility
	GpuVisibilityAccelerationStructuresOutput visOut;
	{
		const Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};

		GpuVisibilityAccelerationStructuresInput in;
		in.m_passesName = "Main TLAS visiblity";
		in.m_lodReferencePoint = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
		in.m_lodDistances = lodDistances;
		in.m_pointOfTest = in.m_lodReferencePoint;
		in.m_testRadius = g_rayTracingExtendedFrustumDistanceCVar.get();
		in.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjection;
		in.m_rgraph = &ctx.m_renderGraphDescr;

		getRenderer().getGpuVisibilityAccelerationStructures().pupulateRenderGraph(in, visOut);

		m_runCtx.m_visibilityHandle = visOut.m_someBufferHandle;
		m_runCtx.m_visibleRenderableIndicesBuff = visOut.m_renderableIndicesBuffer;
	}

	// Create the TLAS
	AccelerationStructureInitInfo initInf("Main TLAS");
	initInf.m_type = AccelerationStructureType::kTopLevel;
	initInf.m_topLevel.m_indirectArgs.m_maxInstanceCount = GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount();
	initInf.m_topLevel.m_indirectArgs.m_instancesBuffer = visOut.m_instancesBuffer.m_buffer;
	initInf.m_topLevel.m_indirectArgs.m_instancesBufferOffset = visOut.m_instancesBuffer.m_offset;
	m_runCtx.m_tlas = GrManager::getSingleton().newAccelerationStructure(initInf);

	// Build the AS
	{
		RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

		const BufferOffsetRange scratchBuff = GpuVisibleTransientMemoryPool::getSingleton().allocate(m_runCtx.m_tlas->getBuildScratchBufferSize());

		m_runCtx.m_tlasHandle = rgraph.importAccelerationStructure(m_runCtx.m_tlas.get(), AccelerationStructureUsageBit::kNone);

		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Build TLAS");
		rpass.newAccelerationStructureDependency(m_runCtx.m_tlasHandle, AccelerationStructureUsageBit::kBuild);
		rpass.newBufferDependency(visOut.m_someBufferHandle, BufferUsageBit::kAccelerationStructureBuild);

		rpass.setWork([this, scratchBuff](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ASBuilder);
			rgraphCtx.m_commandBuffer->buildAccelerationStructure(m_runCtx.m_tlas.get(), scratchBuff.m_buffer, scratchBuff.m_offset);
		});
	}
}

} // end namespace anki
