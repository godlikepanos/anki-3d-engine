// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Texture.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>

namespace anki {

//==============================================================================
Texture::Texture(GrManager* manager)
	: GrObject(manager)
{}

//==============================================================================
Texture::~Texture()
{}

//==============================================================================
class CreateTextureCommand final: public GlCommand
{
public:
	IntrusivePtr<Texture> m_tex;
	TextureInitializer m_init;

	CreateTextureCommand(Texture* tex, const TextureInitializer& init)
		: m_tex(tex)
		, m_init(init)
	{}

	Error operator()(GlState&)
	{
		TextureImpl& impl = m_tex->getImplementation();

		impl.create(m_init);

		GlObject::State oldState = impl.setStateAtomically(
			GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		return ErrorCode::NONE;
	}
};

void Texture::create(const TextureInitializer& init)
{
	m_impl.reset(getAllocator().newInstance<TextureImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>();

	cmdb->getImplementation().pushBackNewCommand<CreateTextureCommand>(
		this, init);
	cmdb->flush();
}

} // end namespace anki
