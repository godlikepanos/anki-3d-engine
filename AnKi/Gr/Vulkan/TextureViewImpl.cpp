// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/TextureViewImpl.h>
#include <AnKi/Gr/Vulkan/TextureImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

namespace anki {

TextureViewImpl::~TextureViewImpl()
{
}

Error TextureViewImpl::init(const TextureViewInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	// Store some stuff
	m_subresource = inf;

	m_tex = inf.m_texture;
	const TextureImpl& tex = static_cast<const TextureImpl&>(*m_tex);
	ANKI_ASSERT(tex.isSubresourceValid(inf));

	// Ask the texture for a view
	m_microImageView = &tex.getOrCreateView(inf);
	m_handle = m_microImageView->getHandle();
	m_texType = m_microImageView->getDerivedTextureType();

	// Create the hash
	Array<U64, 2> toHash = {tex.getUuid(), ptrToNumber(m_handle)};
	m_hash = computeHash(&toHash[0], sizeof(toHash));

	return Error::NONE;
}

U32 TextureViewImpl::getOrCreateBindlessIndex(VkImageLayout layout)
{
	const U32 arrayIdx = (layout == VK_IMAGE_LAYOUT_GENERAL) ? 1 : 0;
	U32& bindlessIdx = m_bindlessIndices[arrayIdx];

	if(bindlessIdx == MAX_U32)
	{
		bindlessIdx = m_microImageView->getOrCreateBindlessIndex(layout, getGrManagerImpl());
	}

	return bindlessIdx;
}

} // end namespace anki
