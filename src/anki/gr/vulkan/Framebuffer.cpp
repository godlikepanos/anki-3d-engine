// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	return FramebufferImpl::newInstanceHelper(manager, init);
}

} // end namespace anki
