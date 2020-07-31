// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup vulkan
/// @{

/// On disk pipeline cache.
class PipelineCache
{
public:
	VkPipelineCache m_cacheHandle = VK_NULL_HANDLE;

	ANKI_USE_RESULT Error init(VkDevice dev, VkPhysicalDevice pdev, CString cacheDir, const ConfigSet& cfg,
							   GrAllocator<U8> alloc);

	void destroy(VkDevice dev, VkPhysicalDevice pdev, GrAllocator<U8> alloc);

private:
	String m_dumpFilename;
	PtrSize m_dumpSize = 0;

	ANKI_USE_RESULT Error destroyInternal(VkDevice dev, VkPhysicalDevice pdev, GrAllocator<U8> alloc);
};
/// @}

} // end namespace anki