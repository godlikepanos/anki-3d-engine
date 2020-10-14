// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/TextureView.h>
#include <anki/gr/vulkan/TextureViewImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

TextureView* TextureView::newInstance(GrManager* manager, const TextureViewInitInfo& init)
{
	TextureViewImpl* impl = manager->getAllocator().newInstance<TextureViewImpl>(manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
	}
	return impl;
}

U32 TextureView::getOrCreateBindlessTextureIndex()
{
	ANKI_VK_SELF(TextureViewImpl);
	ANKI_ASSERT(self.getTextureImpl().computeLayout(TextureUsageBit::ALL_SAMPLED, 0)
				== VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	return self.getOrCreateBindlessIndex(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, DescriptorType::TEXTURE);
}

U32 TextureView::getOrCreateBindlessImageIndex()
{
	ANKI_VK_SELF(TextureViewImpl);
	ANKI_ASSERT(self.getTextureImpl().computeLayout(TextureUsageBit::ALL_IMAGE, 0) == VK_IMAGE_LAYOUT_GENERAL);
	return self.getOrCreateBindlessIndex(VK_IMAGE_LAYOUT_GENERAL, DescriptorType::IMAGE);
}

} // end namespace anki
