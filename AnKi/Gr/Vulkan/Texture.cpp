// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Vulkan/TextureImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

Texture* Texture::newInstance(GrManager* manager, const TextureInitInfo& init)
{
	TextureImpl* impl = anki::newInstance<TextureImpl>(manager->getMemoryPool(), manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(manager->getMemoryPool(), impl);
		impl = nullptr;
	}
	return impl;
}

} // end namespace anki
