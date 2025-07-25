// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/D3D/D3DCommandBufferFactory.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Gr/D3D/D3DGraphicsState.h>
#include <AnKi/Gr/D3D/D3DQueryFactory.h>
#include <AnKi/Gr/D3D/D3DShaderProgram.h>

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

	void postSubmitWork(D3DMicroFence* fence);

private:
	D3D12GraphicsCommandListX* m_cmdList = nullptr; // Cache it.
	U32 m_commandCount = 0;

	MicroCommandBufferPtr m_mcmdb;

	DescriptorState m_descriptors;
	GraphicsStateTracker m_graphicsState;

	StackMemoryPool m_fastPool; // Cache it.

	DynamicArray<TimestampQueryInternalPtr, MemoryPoolPtrWrapper<StackMemoryPool>> m_timestampQueries;
	DynamicArray<PipelineQueryInternalPtr, MemoryPoolPtrWrapper<StackMemoryPool>> m_pipelineQueries;

	const ShaderProgramImpl* m_wgProg = nullptr;

	Bool m_descriptorHeapsBound = false;
	Bool m_debugMarkersEnabled = false;
	Bool m_lineWidthWarningAlreadyShown = false;

	void commandCommon()
	{
		++m_commandCount;
	}

	ANKI_FORCE_INLINE void drawcallCommon()
	{
		commandCommon();

		static_cast<ShaderProgramImpl&>(m_graphicsState.getShaderProgram()).m_graphics.m_pipelineFactory->flushState(m_graphicsState, *m_cmdList);
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
