// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Framebuffer.h>
#include <anki/gr/gl/FramebufferImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Framebuffer::Framebuffer(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

Framebuffer::~Framebuffer()
{
}

void Framebuffer::init(const FramebufferInitInfo& init)
{
	class CreateFramebufferCommand final : public GlCommand
	{
	public:
		FramebufferPtr m_fb;
		FramebufferInitInfo m_init;

		CreateFramebufferCommand(Framebuffer* handle, const FramebufferInitInfo& init)
			: m_fb(handle)
			, m_init(init)
		{
		}

		Error operator()(GlState&)
		{
			FramebufferImpl& impl = *m_fb->m_impl;
			Error err = impl.init(m_init);

			GlObject::State oldState =
				impl.setStateAtomically((err) ? GlObject::State::ERROR : GlObject::State::CREATED);
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	m_impl.reset(getAllocator().newInstance<FramebufferImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());

	cmdb->m_impl->pushBackNewCommand<CreateFramebufferCommand>(this, init);
	cmdb->flush();
}

} // end namespace anki
