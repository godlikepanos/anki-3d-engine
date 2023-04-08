// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/Vulkan/SemaphoreFactory.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Buffer implementation
class FenceImpl final : public Fence
{
public:
	MicroSemaphorePtr m_semaphore; ///< Yes, it's a timeline semaphore and not a VkFence.

	FenceImpl(CString name)
		: Fence(name)
	{
	}

	~FenceImpl()
	{
	}
};
/// @}

} // end namespace anki
