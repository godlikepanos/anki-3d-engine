// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/TextureView.h>
#include <AnKi/Gr/gl/TextureViewImpl.h>
#include <AnKi/Gr/gl/CommandBufferImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

TextureView* TextureView::newInstance(GrManager* manager, const TextureViewInitInfo& init)
{
	class CreateTextureViewCommand final : public GlCommand
	{
	public:
		TextureViewPtr m_view;

		CreateTextureViewCommand(TextureView* view)
			: m_view(view)
		{
		}

		Error operator()(GlState&)
		{
			TextureViewImpl& impl = static_cast<TextureViewImpl&>(*m_view);

			impl.init();

			GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
			(void)oldState;

			return Error::kNone;
		}
	};

	TextureViewImpl* impl = manager->getAllocator().newInstance<TextureViewImpl>(manager, init.getName());
	impl->getRefcount().fetchAdd(1); // Hold a reference in case the command finishes and deletes quickly

	// Need to pre-init because some funcs ask for members and we don't want to serialize
	impl->preInit(init);

	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CreateTextureViewCommand>(impl);
	static_cast<CommandBufferImpl&>(*cmdb).flush();

	return impl;
}

} // end namespace anki
