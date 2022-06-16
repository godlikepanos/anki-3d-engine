// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/FenceFactory.h>
#include <AnKi/Gr/Vulkan/MicroObjectRecycler.h>
#include <AnKi/Util/Ptr.h>

namespace anki {

// Forward
class SwapchainFactory;

/// @addtogroup vulkan
/// @{

/// A wrapper for the swapchain.
class MicroSwapchain
{
	friend class MicroSwapchainPtrDeleter;
	friend class SwapchainFactory;

public:
	VkSwapchainKHR m_swapchain = {};

	DynamicArray<TexturePtr> m_textures;

	MicroSwapchain(SwapchainFactory* factory);

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

	GrAllocator<U8> getAllocator() const;

	void setFence(MicroFencePtr fence)
	{
		m_fence = fence;
	}

	MicroFencePtr& getFence()
	{
		return m_fence;
	}

	/// Interface method.
	void onFenceDone()
	{
		// Do nothing
	}

	VkRenderPass getRenderPass(VkAttachmentLoadOp loadOp) const
	{
		const U idx = (loadOp == VK_ATTACHMENT_LOAD_OP_DONT_CARE) ? RPASS_LOAD_DONT_CARE : RPASS_LOAD_CLEAR;
		return m_rpasses[idx];
	}

private:
	mutable Atomic<I32> m_refcount = {0};
	SwapchainFactory* m_factory = nullptr;

	enum
	{
		RPASS_LOAD_CLEAR,
		RPASS_LOAD_DONT_CARE,
		RPASS_COUNT
	};

	Array<VkRenderPass, RPASS_COUNT> m_rpasses = {};

	MicroFencePtr m_fence;

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
class SwapchainFactory
{
	friend class MicroSwapchainPtrDeleter;
	friend class MicroSwapchain;

public:
	void init(GrManagerImpl* manager, Bool vsync);

	void destroy()
	{
		m_recycler.destroy();
	}

	MicroSwapchainPtr newInstance();

private:
	GrManagerImpl* m_gr = nullptr;
	Bool m_vsync = false;
	MicroObjectRecycler<MicroSwapchain> m_recycler;
};
/// @}

inline void MicroSwapchainPtrDeleter::operator()(MicroSwapchain* s)
{
	ANKI_ASSERT(s);
	s->m_factory->m_recycler.recycle(s);
}

} // end namespace anki
