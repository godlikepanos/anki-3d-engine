// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Texture container.
class TextureImpl : public VulkanObject
{
public:
	TextureImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~TextureImpl();
};
/// @}

} // end namespace anki
