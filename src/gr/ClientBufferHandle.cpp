// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/ClientBufferHandle.h"
#include "anki/gr/ClientBufferImpl.h"
#include "anki/gr/CommandBufferHandle.h"
#include "anki/gr/GlDevice.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
ClientBufferHandle::ClientBufferHandle()
{}

//==============================================================================
ClientBufferHandle::~ClientBufferHandle()
{}

//==============================================================================
Error ClientBufferHandle::create(
	CommandBufferHandle& commands, PtrSize size, void* preallocatedMem)
{
	ANKI_ASSERT(!isCreated());

	using Deleter = GlHandleDefaultDeleter<
		ClientBufferImpl, GlCommandBufferAllocator<ClientBufferImpl>>;

	auto alloc = commands._getAllocator();
	Error err = _createSimple(alloc, Deleter());

	if(!err)
	{
		if(preallocatedMem != nullptr)
		{
			err = _get().create(preallocatedMem, size);
		}
		else
		{
			err = _get().create(alloc, size);

			ANKI_COUNTER_INC(GL_CLIENT_BUFFERS_SIZE, U64(size));
		}
	}

	return err;
}

//==============================================================================
void* ClientBufferHandle::getBaseAddress()
{
	return _get().getBaseAddress();
}

//==============================================================================
PtrSize ClientBufferHandle::getSize() const
{
	return _get().getSize();
}

} // end namespace anki

