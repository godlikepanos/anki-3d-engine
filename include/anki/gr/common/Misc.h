// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Enums.h>

namespace anki
{

enum class TransientBufferType
{
	UNIFORM,
	STORAGE,
	VERTEX,
	TRANSFER,
	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TransientBufferType, inline)

/// Convert buff usage to TransientBufferType.
inline TransientBufferType bufferUsageToTransient(BufferUsageBit bit)
{
	if((bit & BufferUsageBit::UNIFORM_ANY_SHADER) != BufferUsageBit::NONE)
	{
		return TransientBufferType::UNIFORM;
	}
	else if((bit & BufferUsageBit::STORAGE_ANY) != BufferUsageBit::NONE)
	{
		return TransientBufferType::STORAGE;
	}
	else if((bit & BufferUsageBit::VERTEX) != BufferUsageBit::NONE)
	{
		return TransientBufferType::VERTEX;
	}
	else
	{
		ANKI_ASSERT(
			(bit & BufferUsageBit::TRANSFER_SOURCE) != BufferUsageBit::NONE);
		return TransientBufferType::TRANSFER;
	}
}

} // end namespace anki
