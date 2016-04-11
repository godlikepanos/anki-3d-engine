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
ANKI_USE_RESULT VkCompareOp convertCompareOp(CompareOperation ak);

/// Convert format.
ANKI_USE_RESULT VkFormat convertFormat(PixelFormat ak);

/// Convert topology.
ANKI_USE_RESULT VkPrimitiveTopology convertTopology(PrimitiveTopology ak);

/// Convert fill mode.
ANKI_USE_RESULT VkPolygonMode convertFillMode(FillMode ak);

/// Convert cull mode.
ANKI_USE_RESULT VkCullModeFlags convertCullMode(CullMode ak);

/// Convert blend method.
ANKI_USE_RESULT VkBlendFactor convertBlendMethod(BlendMethod ak);

/// Convert blend function.
ANKI_USE_RESULT VkBlendOp convertBlendFunc(BlendFunction ak);

/// Convert color write mask.
inline ANKI_USE_RESULT VkColorComponentFlags convertColorWriteMask(ColorBit ak)
{
	return static_cast<U>(ak);
}
/// @}

} // end namespace anki
