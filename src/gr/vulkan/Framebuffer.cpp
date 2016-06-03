// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Framebuffer.h>
#include <anki/gr/vulkan/FramebufferImpl.h>

namespace anki
{

//==============================================================================
Framebuffer::Framebuffer(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
}

//==============================================================================
Framebuffer::~Framebuffer()
{
}

//==============================================================================
void Framebuffer::init(const FramebufferInitInfo& init)
{
	m_impl.reset(getAllocator().newInstance<FramebufferImpl>(&getManager()));
	if(m_impl->init(init))
	{
		ANKI_LOGF("Cannot recover");
	}
}

} // end namespace anki