// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/TextureViewImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

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
	m_handle = m_microImageView->m_handle;
	m_texType = m_microImageView->m_derivedTextureType;

	// Create the hash
	Array<U64, 2> toHash = {tex.getUuid(), ptrToNumber(m_handle)};
	m_hash = computeHash(&toHash[0], sizeof(toHash));

	return Error::NONE;
}

U32 TextureViewImpl::getOrCreateBindlessIndex(VkImageLayout layout, DescriptorType resourceType)
{
	ANKI_ASSERT(layout == VK_IMAGE_LAYOUT_GENERAL || layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	ANKI_ASSERT(resourceType == DescriptorType::TEXTURE || resourceType == DescriptorType::IMAGE);
	if(resourceType == DescriptorType::IMAGE)
	{
		ANKI_ASSERT(layout == VK_IMAGE_LAYOUT_GENERAL);
	}
	else
	{
		ANKI_ASSERT(layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	ANKI_ASSERT(m_microImageView);

	const U32 arrayIdx = (resourceType == DescriptorType::IMAGE) ? 1 : 0;

	LockGuard<SpinLock> lock(m_microImageView->m_lock);

	U32 outIdx;
	if(m_microImageView->m_bindlessIndices[arrayIdx] != MAX_U32)
	{
		outIdx = m_microImageView->m_bindlessIndices[arrayIdx];
	}
	else
	{
		// Needs binding to the bindless descriptor set

		if(resourceType == DescriptorType::TEXTURE)
		{
			outIdx = getGrManagerImpl().getDescriptorSetFactory().bindBindlessTexture(m_handle, layout);
		}
		else
		{
			outIdx = getGrManagerImpl().getDescriptorSetFactory().bindBindlessImage(m_handle);
		}

		m_microImageView->m_bindlessIndices[arrayIdx] = outIdx;
	}

	return outIdx;
}

} // end namespace anki
