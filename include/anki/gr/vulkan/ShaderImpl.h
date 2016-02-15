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

class ShaderImpl : public VulkanObject
{
public:
	ShaderImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~ShaderImpl();
};
/// @}

} // end namespace anki
