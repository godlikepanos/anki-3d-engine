// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Texture.h>
#include <anki/gr/vulkan/TextureImpl.h>

namespace anki
{

Texture::Texture(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

Texture::~Texture()
{
}

void Texture::init(const TextureInitInfo& init)
{
	m_impl.reset(getAllocator().newInstance<TextureImpl>(&getManager(), getUuid()));

	if(m_impl->init(init, this))
	{
		ANKI_VK_LOGF("Cannot recover");
	}
}

} // end namespace anki
