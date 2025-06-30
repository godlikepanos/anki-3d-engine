// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Gr/BackendCommon/MicroFenceFactory.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Fence wrapper over VkFence.
class MicroFenceImpl
{
public:
	VkFence m_handle = VK_NULL_HANDLE;

	~MicroFenceImpl()
	{
		ANKI_ASSERT(!m_handle);
	}

	operator Bool() const
	{
		return m_handle != 0;
	}

	Bool clientWait(Second seconds)
	{
		ANKI_ASSERT(m_handle);
		const F64 nsf = 1e+9 * seconds;
		const U64 ns = U64(nsf);
		VkResult res;
		ANKI_VK_CHECKF(res = vkWaitForFences(getVkDevice(), 1, &m_handle, true, ns));

		return res != VK_TIMEOUT;
	}

	Bool signaled()
	{
		ANKI_ASSERT(m_handle);
		VkResult status;
		ANKI_VK_CHECKF(status = vkGetFenceStatus(getVkDevice(), m_handle));
		return status == VK_SUCCESS;
	}

	void create()
	{
		ANKI_ASSERT(!m_handle);
		VkFenceCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		ANKI_VK_CHECKF(vkCreateFence(getVkDevice(), &ci, nullptr, &m_handle));
	}

	void destroy()
	{
		ANKI_ASSERT(m_handle);
		vkDestroyFence(getVkDevice(), m_handle, nullptr);
		m_handle = 0;
	}

	void reset()
	{
		ANKI_ASSERT(m_handle);
		ANKI_VK_CHECKF(vkResetFences(getVkDevice(), 1, &m_handle));
	}

	void setName(CString name) const;
};

using VulkanMicroFence = MicroFence<MicroFenceImpl>;

/// Deleter for FencePtr.
class MicroFencePtrDeleter
{
public:
	void operator()(VulkanMicroFence* fence);
};

/// Fence smart pointer.
using MicroFencePtr = IntrusivePtr<VulkanMicroFence, MicroFencePtrDeleter>;

/// A factory of fences.
class FenceFactory : public MakeSingleton<FenceFactory>
{
	friend class MicroFencePtrDeleter;

public:
	/// Create a new fence pointer.
	MicroFencePtr newInstance(CString name = "unnamed")
	{
		return MicroFencePtr(m_factory.newFence(name));
	}

private:
	MicroFenceFactory<MicroFenceImpl> m_factory;

	void deleteFence(VulkanMicroFence* fence)
	{
		m_factory.releaseFence(fence);
	}
};

inline void MicroFencePtrDeleter::operator()(VulkanMicroFence* fence)
{
	ANKI_ASSERT(fence);
	FenceFactory::getSingleton().deleteFence(fence);
}
/// @}

} // end namespace anki
