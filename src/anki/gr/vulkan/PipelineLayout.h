// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/DescriptorSet.h>
#include <anki/util/HashMap.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

class PipelineLayout
{
	friend class PipelineLayoutFactory;

public:
	VkPipelineLayout getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

private:
	VkPipelineLayout m_handle = VK_NULL_HANDLE;
};

/// Creator of pipeline layouts.
class PipelineLayoutFactory
{
public:
	PipelineLayoutFactory() = default;
	~PipelineLayoutFactory() = default;

	void init(GrAllocator<U8> alloc, VkDevice dev)
	{
		m_alloc = alloc;
		m_dev = dev;
	}

	void destroy();

	/// @note It's thread-safe.
	ANKI_USE_RESULT Error newPipelineLayout(const WeakArray<DescriptorSetLayout>& dsetLayouts, PipelineLayout& layout);

private:
	class Layout;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;

	IntrusiveHashMap<U64, Layout> m_layouts;
	Mutex m_layoutsMtx;
};
/// @}

} // end namespace anki
