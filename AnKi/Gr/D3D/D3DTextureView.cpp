// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DTextureView.h>

namespace anki {

TextureView* TextureView::newInstance(const TextureViewInitInfo& init)
{
	TextureViewImpl* impl = anki::newInstance<TextureViewImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

U32 TextureView::getOrCreateBindlessTextureIndex()
{
	ANKI_ASSERT(!"TODO");
	return 0;
}

} // end namespace anki
