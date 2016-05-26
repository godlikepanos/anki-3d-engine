// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Buffer.h>
#include <anki/gr/vulkan/BufferImpl.h>

namespace anki
{

//==============================================================================
Buffer::Buffer(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
}

//==============================================================================
Buffer::~Buffer()
{
}

//==============================================================================
void Buffer::init(PtrSize size, BufferUsageBit usage, BufferAccessBit access)
{
}

//==============================================================================
void* Buffer::map(PtrSize offset, PtrSize range, BufferAccessBit access)
{
}

//==============================================================================
void Buffer::unmap()
{
}

} // end namespace anki