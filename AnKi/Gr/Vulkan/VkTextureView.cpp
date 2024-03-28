// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkTextureView.h>
#include <AnKi/Gr/Vulkan/VkTexture.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>

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
	ANKI_VK_SELF(TextureViewImpl);
	ANKI_ASSERT(self.getTextureImpl().computeLayout(TextureUsageBit::kAllSampled, 0) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	if(self.m_bindlessIndex == kMaxU32)
	{
		self.m_bindlessIndex = self.m_microImageView->getOrCreateBindlessIndex();
	}

	return self.m_bindlessIndex;
}

TextureViewImpl::~TextureViewImpl()
{
}

Error TextureViewImpl::init(const TextureViewInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	// Store some stuff
	m_subresource = inf;

	m_tex.reset(inf.m_texture);
	const TextureImpl& tex = static_cast<const TextureImpl&>(*m_tex);
	ANKI_ASSERT(tex.isSubresourceValid(inf));

	// Ask the texture for a view
	m_microImageView = &tex.getOrCreateView(inf);
	m_handle = m_microImageView->getHandle();
	m_texType = m_microImageView->getDerivedTextureType();

	// Create the hash
	Array<U64, 2> toHash = {tex.getUuid(), ptrToNumber(m_handle)};
	m_hash = computeHash(&toHash[0], sizeof(toHash));

	return Error::kNone;
}

} // end namespace anki
