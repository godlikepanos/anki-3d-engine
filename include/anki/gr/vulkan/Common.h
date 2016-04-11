// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <vulkan/vulkan.h>

namespace anki
{

// Forward
class GrManagerImpl;

/// @addtogroup vulkan
/// @{

/// Check if a vulkan function failed.
#define ANKI_VK_CHECK(x)                                                       \
	do                                                                         \
	{                                                                          \
		VkResult rez = x;                                                      \
		if(rez != VK_SUCCESS)                                                  \
		{                                                                      \
			ANKI_LOGF("Vulkan function failed: %s at (%s:%d %s)",              \
				#x,                                                            \
				ANKI_FILE,                                                     \
				__LINE__,                                                      \
				ANKI_FUNC);                                                    \
		}                                                                      \
	} while(0)

/// Debug initialize a vulkan structure
#if ANKI_DEBUG
#define ANKI_VK_MEMSET_DBG(struct_) memset(&struct_, 0xCC, sizeof(struct_))
#else
#define ANKI_VK_MEMSET_DBG(struct_) ((void)0)
#endif

/// Convert compare op.
VkCompareOp convertCompareOp(CompareOperation ak);

/// Convert format.
VkFormat convertFormat(PixelFormat ak);
/// @}

} // end namespace anki
