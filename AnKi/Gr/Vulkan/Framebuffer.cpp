// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Framebuffer.h>
#include <AnKi/Gr/Vulkan/FramebufferImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

Framebuffer* Framebuffer::newInstance(GrManager* manager, const FramebufferInitInfo& init)
{
	FramebufferImpl* impl = manager->getAllocator().newInstance<FramebufferImpl>(manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
	}
	return impl;
}

} // end namespace anki
