// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlClientBufferHandle.h"
#include "anki/gl/GlClientBuffer.h"
#include "anki/gl/GlCommandBufferHandle.h"
#include "anki/gl/GlDevice.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
GlClientBufferHandle::GlClientBufferHandle()
{}

//==============================================================================
GlClientBufferHandle::GlClientBufferHandle(
	GlCommandBufferHandle& commands, PtrSize size, void* preallocatedMem)
{
	ANKI_ASSERT(!isCreated());

	auto alloc = commands._getAllocator();

	typedef GlHandleDefaultDeleter<
		GlClientBuffer, GlCommandBufferAllocator<GlClientBuffer>> Deleter;

	if(preallocatedMem != nullptr)
	{
		*static_cast<Base*>(this) = Base(
			nullptr, 
			alloc, 
			Deleter(),
			preallocatedMem, 
			size);
	}
	else
	{
		*static_cast<Base*>(this) = Base(
			nullptr, 
			alloc, 
			Deleter(),
			alloc, 
			size);

		ANKI_COUNTER_INC(GL_CLIENT_BUFFERS_SIZE, U64(size));
	}
}

//==============================================================================
GlClientBufferHandle::~GlClientBufferHandle()
{}

//==============================================================================
void* GlClientBufferHandle::getBaseAddress()
{
	return _get().getBaseAddress();
}

//==============================================================================
PtrSize GlClientBufferHandle::getSize() const
{
	return _get().getSize();
}

} // end namespace anki

