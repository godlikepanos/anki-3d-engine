// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Texture.h>
#include <anki/gr/vulkan/TextureImpl.h>

namespace anki
{

//==============================================================================
Texture::Texture(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
}

//==============================================================================
Texture::~Texture()
{
}

//==============================================================================
void Texture::init(const TextureInitInfo& init)
{
	m_impl.reset(getAllocator().newInstance<TextureImpl>(&getManager()));

	if(m_impl->init(init, this))
	{
		ANKI_LOGF("Cannot recover");
	}
}

} // end namespace anki
