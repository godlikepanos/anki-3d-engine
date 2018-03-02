// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

	TextureViewImpl(GrManager* manager, CString name)
		: TextureView(manager, name)
	{
	}

	~TextureViewImpl();

	ANKI_USE_RESULT Error init(const TextureViewInitInfo& inf);

	VkImageSubresourceRange getVkImageSubresourceRange() const
	{
		VkImageSubresourceRange out;
		static_cast<const TextureImpl&>(*m_tex).computeVkImageSubresourceRange(getSubresource(), out);
		return out;
	}
};
/// @}

} // end namespace anki
