// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/D3D/D3DTexture.h>

namespace anki {

Texture* Texture::newInstance(const TextureInitInfo& init)
{
	TextureImpl* impl = anki::newInstance<TextureImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

TextureImpl::~TextureImpl()
{
}

Error TextureImpl::init(const TextureInitInfo& inf)
{
	ANKI_ASSERT(!"TODO");
	return Error::kNone;
}

} // end namespace anki
