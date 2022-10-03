// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

void AccelerationStructureBuilder::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_TLAS);

	// Get some things
	ANKI_ASSERT(ctx.m_renderQueue->m_rayTracingQueue);
	ConstWeakArray<RayTracingInstanceQueueElement> instanceElements =
		ctx.m_renderQueue->m_rayTracingQueue->m_rayTracingInstances;
	const U32 instanceCount = instanceElements.getSize();
	ANKI_ASSERT(instanceCount > 0);

	// Create the instances. Allocate but not construct to save some CPU time
	void* instancesMem = ctx.m_tempPool->allocate(sizeof(AccelerationStructureInstance) * instanceCount,
												  alignof(AccelerationStructureInstance));
	WeakArray<AccelerationStructureInstance> instances(static_cast<AccelerationStructureInstance*>(instancesMem),
													   instanceCount);

	for(U32 instanceIdx = 0; instanceIdx < instanceCount; ++instanceIdx)
	{
		const RayTracingInstanceQueueElement& element = instanceElements[instanceIdx];

		// Init instance
		AccelerationStructureInstance& out = instances[instanceIdx];
		::new(&out) AccelerationStructureInstance();
		out.m_bottomLevel.reset(element.m_bottomLevelAccelerationStructure);
		memcpy(&out.m_transform, &element.m_transform, sizeof(out.m_transform));
		out.m_hitgroupSbtRecordIndex = instanceIdx;
		out.m_mask = 0xFF;
	}

	// Create the TLAS
	AccelerationStructureInitInfo initInf("MainTlas");
	initInf.m_type = AccelerationStructureType::kTopLevel;
	initInf.m_topLevel.m_instances = instances;
	m_runCtx.m_tlas = getGrManager().newAccelerationStructure(initInf);

	// Need a cleanup
	for(U32 instanceIdx = 0; instanceIdx < instanceCount; ++instanceIdx)
	{
		instances[instanceIdx].m_bottomLevel.reset(nullptr);
	}

	// Build the job
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_tlasHandle = rgraph.importAccelerationStructure(m_runCtx.m_tlas, AccelerationStructureUsageBit::kNone);
	ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("BuildTlas");
	rpass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(R_TLAS);
		rgraphCtx.m_commandBuffer->buildAccelerationStructure(m_runCtx.m_tlas);
	});

	rpass.newAccelerationStructureDependency(m_runCtx.m_tlasHandle, AccelerationStructureUsageBit::kBuild);
}

} // end namespace anki
