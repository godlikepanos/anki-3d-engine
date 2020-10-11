// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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

	VkImageView getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	U64 getHash() const
	{
		ANKI_ASSERT(m_hash);
		return m_hash;
	}

	const TextureImpl& getTextureImpl() const
	{
		return static_cast<const TextureImpl&>(*m_tex);
	}

	/// @param resourceType Texture or image.
	/// @note It's thread-safe.
	U32 getOrCreateBindlessIndex(VkImageLayout layout, DescriptorType resourceType);

private:
	VkImageView m_handle = {}; /// Cache the handle.

	/// This is a hash that depends on the Texture and the VkImageView. It's used as a replacement of
	/// TextureView::m_uuid since it creates less unique IDs.
	U64 m_hash = 0;

	const MicroImageView* m_microImageView = nullptr;

	TexturePtr m_tex; ///< Hold a reference.
};
/// @}

} // end namespace anki
