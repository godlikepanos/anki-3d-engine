// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/TextureView.h>
#include <anki/gr/vulkan/VulkanObject.h>

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

	TextureViewImpl(GrManager* manager)
		: TextureView(manager)
	{
	}

	~TextureViewImpl();

	ANKI_USE_RESULT Error init(const TextureViewInitInfo& inf);
};
/// @}

} // end namespace anki
