// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/TextureView.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/TextureImpl.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Texture view implementation.
class TextureViewImpl final : public TextureView, public VulkanObject<TextureView, TextureViewImpl>
{
public:
	VkImageView m_handle = {};
	TexturePtr m_tex; ///< Hold a reference.

	/// This is a hash that depends on the Texture and the VkImageView. It's used as a replacement of
	/// TextureView::m_uuid since it creates less unique IDs.
	U64 m_hash = 0;

	TextureViewImpl(GrManager* manager)
		: TextureView(manager)
	{
	}

	~TextureViewImpl();

	ANKI_USE_RESULT Error init(const TextureViewInitInfo& inf);

	TextureSubresourceInfo getSubresource() const
	{
		TextureSubresourceInfo out;
		out.m_baseMipmap = m_baseMip;
		out.m_mipmapCount = m_mipCount;
		out.m_baseLayer = m_baseLayer;
		out.m_layerCount = m_layerCount;
		out.m_baseFace = m_baseFace;
		out.m_faceCount = m_faceCount;
		out.m_depthStencilAspect = m_aspect;
		return out;
	}

	VkImageSubresourceRange getVkImageSubresourceRange() const
	{
		VkImageSubresourceRange out;
		static_cast<const TextureImpl&>(*m_tex).computeVkImageSubresourceRange(getSubresource(), out);
		return out;
	}
};
/// @}

} // end namespace anki
