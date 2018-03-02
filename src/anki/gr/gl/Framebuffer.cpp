// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Framebuffer.h>
#include <anki/gr/gl/FramebufferImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Framebuffer* Framebuffer::newInstance(GrManager* manager, const FramebufferInitInfo& init)
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
			FramebufferImpl& impl = static_cast<FramebufferImpl&>(*m_fb);
			Error err = impl.init(m_init);

			GlObject::State oldState =
				impl.setStateAtomically((err) ? GlObject::State::ERROR : GlObject::State::CREATED);
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	FramebufferImpl* impl = manager->getAllocator().newInstance<FramebufferImpl>(manager, init.getName());
	impl->getRefcount().fetchAdd(1); // Hold a reference in case the command finishes and deletes quickly

	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CreateFramebufferCommand>(impl, init);
	static_cast<CommandBufferImpl&>(*cmdb).flush();

	return impl;
}

} // end namespace anki
