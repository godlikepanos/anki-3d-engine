// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Fence.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/FenceFactory.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Buffer implementation
class FenceImpl final : public Fence, public VulkanObject<Fence, FenceImpl>
{
public:
	MicroFencePtr m_fence;

	FenceImpl(GrManager* manager, CString name)
		: Fence(manager, name)
	{
	}

	~FenceImpl()
	{
	}
};
/// @}

} // end namespace anki
