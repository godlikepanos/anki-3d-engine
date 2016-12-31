// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Texture.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Texture::Texture(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

Texture::~Texture()
{
}

class CreateTextureCommand final : public GlCommand
{
public:
	IntrusivePtr<Texture> m_tex;
	TextureInitInfo m_init;

	CreateTextureCommand(Texture* tex, const TextureInitInfo& init)
		: m_tex(tex)
		, m_init(init)
	{
	}

	Error operator()(GlState&)
	{
		TextureImpl& impl = *m_tex->m_impl;

		impl.init(m_init);

		GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		return ErrorCode::NONE;
	}
};

void Texture::init(const TextureInitInfo& init)
{
	ANKI_ASSERT(textureInitInfoValid(init));
	m_impl.reset(getAllocator().newInstance<TextureImpl>(&getManager()));

	// Need to pre-init because some funcs ask for members and we don't want
	// to serialize
	m_impl->preInit(init);

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());

	cmdb->m_impl->pushBackNewCommand<CreateTextureCommand>(this, init);
	cmdb->flush();
}

} // end namespace anki
