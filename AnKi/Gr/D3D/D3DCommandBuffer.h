// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/D3D/D3DCommandBufferFactory.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Gr/D3D/D3DGraphicsState.h>
#include <AnKi/Gr/D3D/D3DQueryFactory.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Command buffer implementation.
class CommandBufferImpl final : public CommandBuffer
{
	friend class CommandBuffer;

public:
	CommandBufferImpl(CString name)
		: CommandBuffer(name)
	{
	}

	~CommandBufferImpl();

	Error init(const CommandBufferInitInfo& init);

	MicroCommandBuffer& getMicroCommandBuffer()
	{
		return *m_mcmdb;
	}

	void postSubmitWork(MicroFence* fence);

private:
	D3D12GraphicsCommandListX* m_cmdList = nullptr; // Cache it.
	U32 m_commandCount = 0;

	MicroCommandBufferPtr m_mcmdb;

	DescriptorState m_descriptors;
	GraphicsStateTracker m_graphicsState;

	StackMemoryPool* m_fastPool = nullptr; // Cache it.

	DynamicArray<QueryHandle, MemoryPoolPtrWrapper<StackMemoryPool>> m_timestampQueries;

	Bool m_descriptorHeapsBound = false;
	Bool m_debugMarkersEnabled = false;

	void commandCommon()
	{
		++m_commandCount;
	}

	ANKI_FORCE_INLINE void drawcallCommon()
	{
		commandCommon();

		m_graphicsState.getShaderProgram().m_graphics.m_pipelineFactory->flushState(m_graphicsState, *m_cmdList);
		m_descriptors.flush(*m_cmdList);
	}

	ANKI_FORCE_INLINE void dispatchCommon()
	{
		commandCommon();
		m_descriptors.flush(*m_cmdList);
	}
};
/// @}

} // end namespace anki
