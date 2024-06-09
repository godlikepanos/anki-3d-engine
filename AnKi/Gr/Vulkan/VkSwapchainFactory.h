// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkFenceFactory.h>
#include <AnKi/Gr/BackendCommon/MicroObjectRecycler.h>
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

	GrDynamicArray<TexturePtr> m_textures;

	MicroSwapchain();

	~MicroSwapchain();

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

	void setFence(MicroFence* fence)
	{
		m_fence.reset(fence);
	}

	MicroFence* getFence() const
	{
		return m_fence.tryGet();
	}

	/// Interface method.
	void onFenceDone()
	{
		// Do nothing
	}

private:
	MicroFencePtr m_fence;
	mutable Atomic<I32> m_refcount = {0};

	Error initInternal();
};

/// Deleter for MicroSwapchainPtr smart pointer.
class MicroSwapchainPtrDeleter
{
public:
	void operator()(MicroSwapchain* x);
};

/// MicroSwapchain smart pointer.
using MicroSwapchainPtr = IntrusivePtr<MicroSwapchain, MicroSwapchainPtrDeleter>;

/// Swapchain factory.
class SwapchainFactory : public MakeSingleton<SwapchainFactory>
{
	friend class MicroSwapchainPtrDeleter;
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

inline void MicroSwapchainPtrDeleter::operator()(MicroSwapchain* s)
{
	ANKI_ASSERT(s);
	SwapchainFactory::getSingleton().m_recycler.recycle(s);
}

} // end namespace anki
