// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/Common.h>

namespace anki {

// Forward
class ConfigSet;

/// @addtogroup vulkan
/// @{

/// On disk pipeline cache.
class PipelineCache
{
public:
	VkPipelineCache m_cacheHandle = VK_NULL_HANDLE;

	Error init(VkDevice dev, VkPhysicalDevice pdev, CString cacheDir, const ConfigSet& cfg, HeapMemoryPool& pool);

	void destroy(VkDevice dev, VkPhysicalDevice pdev, HeapMemoryPool& pool);

private:
	String m_dumpFilename;
	PtrSize m_dumpSize = 0;

	Error destroyInternal(VkDevice dev, VkPhysicalDevice pdev, HeapMemoryPool& pool);
};
/// @}

} // end namespace anki
