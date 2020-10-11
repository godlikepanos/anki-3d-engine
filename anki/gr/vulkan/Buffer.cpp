// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Buffer.h>
#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Buffer* Buffer::newInstance(GrManager* manager, const BufferInitInfo& init)
{
	BufferImpl* impl = manager->getAllocator().newInstance<BufferImpl>(manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
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

} // end namespace anki
