// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>
#include <AnKi/Gr/Vulkan/VkSemaphoreFactory.h>
#include <AnKi/Util/Ptr.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// A wrapper for the swapchain.
class MicroSwapchain
{
	friend class MicroSwapchainPtrDeleter;
	friend class SwapchainFactory;

public:
	VkSwapchainKHR m_swapchain = {};

	GrDynamicArray<TextureInternalPtr> m_textures;
	GrDynamicArray<MicroSemaphorePtr> m_acquireSemaphores;
	GrDynamicArray<MicroSemaphorePtr> m_renderSemaphores; ///< Signaled by the operation that renders to a presentable image.

	MicroSwapchain();

	~MicroSwapchain();

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	void release()
	{
		if(m_refcount.fetchSub(1) == 1)
		{
			releaseInternal();
		}
	}

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

private:
	mutable Atomic<I32> m_refcount = {0};

	Error initInternal();

	void releaseInternal();
};

/// MicroSwapchain smart pointer.
using MicroSwapchainPtr = IntrusiveNoDelPtr<MicroSwapchain>;

/// Swapchain factory.
class SwapchainFactory : public MakeSingleton<SwapchainFactory>
{
	friend class MicroSwapchain;

public:
	SwapchainFactory(Bool vsync)
	{
		m_vsync = vsync;
	}

	~SwapchainFactory()
	{
		m_recycler.destroy();
	}

	MicroSwapchainPtr newInstance();

private:
	Bool m_vsync = false;
	MicroObjectRecycler<MicroSwapchain> m_recycler;
};
/// @}

} // end namespace anki
