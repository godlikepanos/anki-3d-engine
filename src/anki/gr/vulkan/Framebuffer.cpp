// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Framebuffer.h>
#include <anki/gr/vulkan/FramebufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

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
