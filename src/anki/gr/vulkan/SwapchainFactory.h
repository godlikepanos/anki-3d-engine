// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/FenceFactory.h>
#include <anki/gr/vulkan/MicroObjectRecycler.h>
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
	friend class MicroSwapchainPtrDeleter;
	friend class SwapchainFactory;

public:
	VkSwapchainKHR m_swapchain = {};

	Array<TexturePtr, MAX_FRAMES_IN_FLIGHT> m_textures;

	MicroSwapchain(SwapchainFactory* factory);

	~MicroSwapchain();

	Atomic<U32>& getRefcount()
	{
		return m_refcount;
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

	VkRenderPass getRenderPass(VkAttachmentLoadOp loadOp) const
	{
		const U idx = (loadOp == VK_ATTACHMENT_LOAD_OP_DONT_CARE) ? RPASS_LOAD_DONT_CARE : RPASS_LOAD_CLEAR;
		return m_rpasses[idx];
	}

private:
	Atomic<U32> m_refcount = {0};
	SwapchainFactory* m_factory = nullptr;

	enum
	{
		RPASS_LOAD_CLEAR,
		RPASS_LOAD_DONT_CARE,
		RPASS_COUNT
	};

	Array<VkRenderPass, RPASS_COUNT> m_rpasses = {};

	MicroFencePtr m_fence;

	ANKI_USE_RESULT Error initInternal();
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
	Bool8 m_vsync = false;
	MicroObjectRecycler<MicroSwapchain> m_recycler;
};
/// @}

inline void MicroSwapchainPtrDeleter::operator()(MicroSwapchain* s)
{
	ANKI_ASSERT(s);
	s->m_factory->m_recycler.recycle(s);
}

} // end namespace anki
