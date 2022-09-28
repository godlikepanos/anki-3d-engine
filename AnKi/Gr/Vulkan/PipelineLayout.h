// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/DescriptorSet.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

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

	void init(HeapMemoryPool* pool, VkDevice dev)
	{
		ANKI_ASSERT(pool);
		m_pool = pool;
		m_dev = dev;
	}

	void destroy();

	/// @note It's thread-safe.
	Error newPipelineLayout(const WeakArray<DescriptorSetLayout>& dsetLayouts, U32 pushConstantsSize,
							PipelineLayout& layout);

private:
	HeapMemoryPool* m_pool = nullptr;
	VkDevice m_dev = VK_NULL_HANDLE;

	HashMap<U64, VkPipelineLayout> m_layouts;
	Mutex m_layoutsMtx;
};
/// @}

} // end namespace anki
