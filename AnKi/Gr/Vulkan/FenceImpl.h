// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/Vulkan/VulkanObject.h>
#include <AnKi/Gr/Vulkan/SemaphoreFactory.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Buffer implementation
class FenceImpl final : public Fence, public VulkanObject<Fence, FenceImpl>
{
public:
	MicroSemaphorePtr m_semaphore; ///< Yes, it's a timeline semaphore and not a VkFence.

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
