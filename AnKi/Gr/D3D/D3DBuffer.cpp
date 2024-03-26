// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Vulkan/BufferImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

Buffer* Buffer::newInstance(const BufferInitInfo& init)
{
	BufferImpl* impl = anki::newInstance<BufferImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

void* Buffer::map(PtrSize offset, PtrSize range, BufferMapAccessBit access)
{
	ANKI_ASSERT(!"TODO");
	return nullptr;
}

void Buffer::unmap()
{
	ANKI_ASSERT(!"TODO");
}

void Buffer::flush(PtrSize offset, PtrSize range) const
{
	ANKI_ASSERT(!"TODO");
}

void Buffer::invalidate(PtrSize offset, PtrSize range) const
{
	ANKI_ASSERT(!"TODO");
}

BufferImpl::~BufferImpl()
{
}

Error BufferImpl::init(const BufferInitInfo& inf)
{
	ANKI_ASSERT(!"TODO");
	return Error::kNone;
}

} // end namespace anki
