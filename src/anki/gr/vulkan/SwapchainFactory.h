// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/FenceFactory.h>
#include <anki/util/Ptr.h>

namespace anki
{

// Forward
class SwapchainFactory;

/// @addtogroup vulkan
/// @{

/// A wrapper for the swapchain.
class MicroSwapchain
{
public:
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

	Array<VkImage, MAX_FRAMES_IN_FLIGHT> m_image = {};
	Array<VkImageView, MAX_FRAMES_IN_FLIGHT> m_imageView = {};

	MicroSwapchain(SwapchainFactory* factory);

	~MicroSwapchain();

	Atomic<U32>& getRefcount()
	{
		return m_refcount;
	}

	GrAllocator<U8> getAllocator() const;

private:
	Atomic<U32> m_refcount = {0};
	SwapchainFactory* m_factory = nullptr;
};

/// Deleter for MicroSwapchainPtr smart pointer.
class MicroSwapchainPtrDeleter
{
public:
	void operator()(MicroSwapchain* x);
};

/// MicroSwapchain smart pointer.
using MicroSwapchainPtr = IntrusivePtr<MicroSwapchain, MicroSwapchainPtrDeleter>;
/// @}

} // end namespace anki