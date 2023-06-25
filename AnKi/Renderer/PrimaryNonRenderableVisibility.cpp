// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Scene/ContiguousArrayAllocator.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>

namespace anki {

void PrimaryNonRenderableVisibility::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx = {};

	for(GpuSceneNonRenderableObjectType type : EnumIterable<GpuSceneNonRenderableObjectType>())
	{
		const GpuSceneContiguousArrayType arrayType = gpuSceneNonRenderableObjectTypeToGpuSceneContiguousArrayType(type);
		const U32 objCount = GpuSceneContiguousArrays::getSingleton().getElementCount(arrayType);

		if(objCount == 0)
		{
			continue;
		}

		GpuVisibilityNonRenderablesInput in;
		in.m_passesName = "NonRenderableVisibility";
		in.m_objectType = type;
		in.m_viewProjectionMat = ctx.m_matrices.m_viewProjection;
		in.m_hzbRt = nullptr; // TODO
		in.m_rgraph = &rgraph;

		const GpuSceneNonRenderableObjectTypeWithFeedback feedbackType = toGpuSceneNonRenderableObjectTypeWithFeedback(type);
		if(feedbackType != GpuSceneNonRenderableObjectTypeWithFeedback::kCount)
		{
			// Read feedback UUIDs from the GPU
			DynamicArray<U32, MemoryPoolPtrWrapper<StackMemoryPool>> readbackData(ctx.m_tempPool);
			getRenderer().getReadbackManager().readMostRecentData(m_readbacks[feedbackType], readbackData);

			if(readbackData.getSize())
			{
				ANKI_ASSERT(readbackData.getSize() > 1);
				const U32 uuidCount = readbackData[0];

				if(uuidCount)
				{
					m_runCtx.m_uuids[feedbackType] = WeakArray<U32>(&readbackData[1], readbackData[0]);

					// Transfer ownership
					WeakArray<U32> dummy;
					readbackData.moveAndReset(dummy);
				}
			}

			// Allocate feedback buffer for this frame
			in.m_cpuFeedbackBuffer.m_range = (objCount + 1) * sizeof(U32);
			getRenderer().getReadbackManager().allocateData(m_readbacks[feedbackType], in.m_cpuFeedbackBuffer.m_range,
															in.m_cpuFeedbackBuffer.m_buffer, in.m_cpuFeedbackBuffer.m_offset);
		}

		GpuVisibilityNonRenderablesOutput out;
		getRenderer().getGpuVisibilityNonRenderables().populateRenderGraph(in, out);

		m_runCtx.m_visOutBufferHandle[type] = out.m_bufferHandle;
	}
}

} // end namespace anki
