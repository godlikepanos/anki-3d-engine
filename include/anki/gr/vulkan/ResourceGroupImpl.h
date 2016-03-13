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

/// Resource group implementation.
class ResourceGroupImpl : public VulkanObject
{
public:
	ResourceGroupImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~ResourceGroupImpl();
};
/// @}

} // end namespace anki
