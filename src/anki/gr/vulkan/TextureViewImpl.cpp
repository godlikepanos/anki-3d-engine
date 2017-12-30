// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/TextureViewImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>

namespace anki
{

TextureViewImpl::~TextureViewImpl()
{
}

Error TextureViewImpl::init(const TextureViewInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	// Store some stuff
	m_aspect = inf.m_depthStencilAspect;
	m_baseMip = inf.m_baseMipmap;
	m_mipCount = inf.m_mipmapCount;
	m_baseLayer = inf.m_baseLayer;
	m_layerCount = inf.m_layerCount;
	m_baseFace = inf.m_baseFace;
	m_faceCount = inf.m_faceCount;
	m_subresource = inf;

	m_tex = inf.m_texture;
	const TextureImpl& tex = static_cast<const TextureImpl&>(*m_tex);
	ANKI_ASSERT(tex.isSubresourceValid(inf));

	m_texType = tex.getTextureType();

	// Compute the VkImageViewCreateInfo
	VkImageViewCreateInfo viewCi;
	memcpy(&viewCi, &tex.m_viewCreateInfoTemplate, sizeof(viewCi)); // Memcpy because it will be hashed
	viewCi.subresourceRange.aspectMask = convertImageAspect(m_aspect);
	viewCi.subresourceRange.baseMipLevel = inf.m_baseMipmap;
	viewCi.subresourceRange.levelCount = inf.m_mipmapCount;

	const U faceCount = textureTypeIsCube(tex.getTextureType()) ? 6 : 1;
	viewCi.subresourceRange.baseArrayLayer = inf.m_baseLayer * faceCount + inf.m_baseFace;
	viewCi.subresourceRange.layerCount = inf.m_layerCount * inf.m_faceCount;

	// Fixup the image view type
	if(textureTypeIsCube(m_texType) && inf.m_faceCount != 6)
	{
		m_texType = TextureType::_2D;
		viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
	}

	// Ask the texture for a view
	m_handle = tex.getOrCreateView(viewCi);

	// Create the hash
	Array<U64, 2> toHash = {{tex.getUuid(), ptrToNumber(m_handle)}};
	m_hash = computeHash(&toHash[0], sizeof(toHash));

	return Error::NONE;
}

} // end namespace anki
