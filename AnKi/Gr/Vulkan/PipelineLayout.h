// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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

	void destroy();

	/// @note It's thread-safe.
	Error newPipelineLayout(const WeakArray<DescriptorSetLayout>& dsetLayouts, U32 pushConstantsSize,
							PipelineLayout& layout);

private:
	GrHashMap<U64, VkPipelineLayout> m_layouts;
	Mutex m_layoutsMtx;
};
/// @}

} // end namespace anki
