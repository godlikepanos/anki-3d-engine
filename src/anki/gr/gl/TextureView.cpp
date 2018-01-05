// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/TextureView.h>
#include <anki/gr/gl/TextureViewImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

TextureView* TextureView::newInstance(GrManager* manager, const TextureViewInitInfo& init)
{
	class CreateTextureViewCommand final : public GlCommand
	{
	public:
		TextureViewPtr m_tex;

		CreateTextureCommand(TextureView* tex)
			: m_tex(tex)
		{
		}

		Error operator()(GlState&)
		{
			TextureImpl& impl = static_cast<TextureImpl&>(*m_tex);

			impl.init();

			GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
			(void)oldState;

			return Error::NONE;
		}
	};

	TextureViewImpl* impl = manager->getAllocator().newInstance<TextureViewImpl>(manager);

	// Need to pre-init because some funcs ask for members and we don't want to serialize
	impl->preInit(init);

	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CreateTextureViewCommand>(impl);
	static_cast<CommandBufferImpl&>(*cmdb).flush();

	return impl;
}

} // end namespace anki
