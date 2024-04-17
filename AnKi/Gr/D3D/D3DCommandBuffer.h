// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/D3D/D3DCommon.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Command buffer implementation.
class CommandBufferImpl final : public CommandBuffer
{
public:
	CommandBufferImpl(CString name)
		: CommandBuffer(name)
	{
	}

	~CommandBufferImpl();

	Error init(const CommandBufferInitInfo& init);

private:
	ID3D12CommandAllocator* m_cmdAllocator = nullptr;
	ID3D12GraphicsCommandList7* m_cmdList = nullptr;
};
/// @}

} // end namespace anki
