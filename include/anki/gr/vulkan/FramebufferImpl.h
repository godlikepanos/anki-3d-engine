// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Framebuffer implementation.
class FramebufferImpl : public VulkanObject
{
public:
	FramebufferImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~FramebufferImpl();
};
/// @}

} // end namespace anki
