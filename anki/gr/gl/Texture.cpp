// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Texture.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Texture* Texture::newInstance(GrManager* manager, const TextureInitInfo& init)
{
	class CreateTextureCommand final : public GlCommand
	{
	public:
		TexturePtr m_tex;
		TextureInitInfo m_init;

		CreateTextureCommand(Texture* tex, const TextureInitInfo& init)
			: m_tex(tex)
			, m_init(init)
		{
		}

		Error operator()(GlState&)
		{
			TextureImpl& impl = static_cast<TextureImpl&>(*m_tex);

			impl.init(m_init);

			GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
			(void)oldState;

			return Error::NONE;
		}
	};

	TextureImpl* impl = manager->getAllocator().newInstance<TextureImpl>(manager, init.getName());
	impl->getRefcount().fetchAdd(1); // Hold a reference in case the command finishes and deletes quickly

	// Need to pre-init because some funcs ask for members and we don't want to serialize
	impl->preInit(init);

	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CreateTextureCommand>(impl, init);
	static_cast<CommandBufferImpl&>(*cmdb).flush();

	return impl;
}

} // end namespace anki
