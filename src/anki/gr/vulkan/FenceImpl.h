// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/FenceFactory.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Buffer implementation
class FenceImpl : public VulkanObject
{
public:
	MicroFencePtr m_fence;

	FenceImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~FenceImpl()
	{
	}
};
/// @}

} // end namespace anki
