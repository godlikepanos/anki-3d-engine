// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/App.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>

namespace anki {

void AccelerationStructureBuilder::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(ASBuilder);

	// Do visibility
	GpuVisibilityAccelerationStructuresOutput visOut;
	{
		const Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar, g_lod1MaxDistanceCVar};

		GpuVisibilityAccelerationStructuresInput in;
		in.m_passesName = "Main TLAS visiblity";
		in.m_lodReferencePoint = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
		in.m_lodDistances = lodDistances;
		in.m_pointOfTest = in.m_lodReferencePoint;
		in.m_testRadius = g_rayTracingExtendedFrustumDistanceCVar;
		in.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjection;
		in.m_rgraph = &ctx.m_renderGraphDescr;

		getRenderer().getGpuVisibilityAccelerationStructures().pupulateRenderGraph(in, visOut);

		m_runCtx.m_dependency = visOut.m_dependency;
		m_runCtx.m_visibleRenderablesBuff = visOut.m_renderablesBuffer;
		m_runCtx.m_buildSbtIndirectArgsBuff = visOut.m_buildSbtIndirectArgsBuffer;
	}

	// Create the TLAS
	AccelerationStructureInitInfo initInf("Main TLAS");
	initInf.m_type = AccelerationStructureType::kTopLevel;
	initInf.m_topLevel.m_instanceCount = GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount();
	initInf.m_topLevel.m_instancesBuffer = visOut.m_instancesBuffer;
	m_runCtx.m_tlas = GrManager::getSingleton().newAccelerationStructure(initInf);

	// Build the AS
	{
		RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

		const BufferView scratchBuff = GpuVisibleTransientMemoryPool::getSingleton().allocate(m_runCtx.m_tlas->getBuildScratchBufferSize(), 1);

		m_runCtx.m_tlasHandle = rgraph.importAccelerationStructure(m_runCtx.m_tlas.get(), AccelerationStructureUsageBit::kNone);

		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Build TLAS");
		rpass.newAccelerationStructureDependency(m_runCtx.m_tlasHandle, AccelerationStructureUsageBit::kBuild);
		rpass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kAccelerationStructureBuild);

		rpass.setWork([this, scratchBuff](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ASBuilder);
			rgraphCtx.m_commandBuffer->buildAccelerationStructure(m_runCtx.m_tlas.get(), scratchBuff);
		});
	}
}

} // end namespace anki
