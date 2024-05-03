// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/D3D/D3DCommandBufferFactory.h>

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

private:
	ID3D12GraphicsCommandList7* m_cmdList = nullptr; // Cache it.
	U32 m_commandCount = 0;

	StackMemoryPool* m_fastPool = nullptr; // Cache it.
	MicroCommandBufferPtr m_mcmdb;

	void commandCommon();
};
/// @}

} // end namespace anki
