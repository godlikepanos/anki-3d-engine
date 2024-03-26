// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/CommandBuffer.h>

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

	~CommandBufferImpl()
	{
	}

	Error init(const CommandBufferInitInfo& init);
};
/// @}

} // end namespace anki
