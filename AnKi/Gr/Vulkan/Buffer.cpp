// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Vulkan/BufferImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

Buffer* Buffer::newInstance(GrManager* manager, const BufferInitInfo& init)
{
	BufferImpl* impl = anki::newInstance<BufferImpl>(manager->getMemoryPool(), manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(manager->getMemoryPool(), impl);
		impl = nullptr;
	}
	return impl;
}

void* Buffer::map(PtrSize offset, PtrSize range, BufferMapAccessBit access)
{
	ANKI_VK_SELF(BufferImpl);
	return self.map(offset, range, access);
}

void Buffer::unmap()
{
	ANKI_VK_SELF(BufferImpl);
	self.unmap();
}

void Buffer::flush(PtrSize offset, PtrSize range) const
{
	ANKI_VK_SELF_CONST(BufferImpl);
	self.flush(offset, range);
}

void Buffer::invalidate(PtrSize offset, PtrSize range) const
{
	ANKI_VK_SELF_CONST(BufferImpl);
	self.invalidate(offset, range);
}

} // end namespace anki
