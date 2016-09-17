// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

#if ANKI_OS == ANKI_OS_LINUX
#define VK_USE_PLATFORM_XCB_KHR 1
#elif ANKI_OS == ANKI_OS_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR 1
#else
#error TODO
#endif
#include <vulkan/vulkan.h>

// I wonder whose stupid idea was to pollute the global namespace with defines like ERROR, near and far
#if ANKI_OS == ANKI_OS_WINDOWS
#undef ERROR
#undef near
#undef far
#endif

namespace anki
{

// Forward
class GrManagerImpl;

/// @addtogroup vulkan
/// @{

/// Check if a vulkan function failed. It will abort on failure.
#define ANKI_VK_CHECKF(x)                                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		VkResult rez;                                                                                                  \
		if((rez = (x)) < 0)                                                                                            \
		{                                                                                                              \
			ANKI_LOGF("Vulkan function failed (VkResult: %d): %s", rez, #x);                                           \
		}                                                                                                              \
	} while(0)

/// Check if a vulkan function failed.
#define ANKI_VK_CHECK(x)                                                                                               \
	do                                                                                                                 \
	{                                                                                                                  \
		VkResult rez;                                                                                                  \
		if((rez = (x)) < 0)                                                                                            \
		{                                                                                                              \
			ANKI_LOGE("Vulkan function failed (VkResult: %d): %s", rez, #x);                                           \
			return ErrorCode::FUNCTION_FAILED;                                                                         \
		}                                                                                                              \
	} while(0)

ANKI_USE_RESULT inline Bool formatIsDepthStencil(PixelFormat fmt)
{
	if(fmt.m_components == ComponentFormat::D16 || fmt.m_components == ComponentFormat::D24
		|| fmt.m_components == ComponentFormat::D32)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/// Convert compare op.
ANKI_USE_RESULT VkCompareOp convertCompareOp(CompareOperation ak);

/// Convert format.
ANKI_USE_RESULT VkFormat convertFormat(PixelFormat ak);

/// Get format aspect mask.
ANKI_USE_RESULT VkImageAspectFlags convertImageAspect(PixelFormat ak);

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

/// Convert load op.
ANKI_USE_RESULT VkAttachmentLoadOp convertLoadOp(AttachmentLoadOperation ak);

/// Convert store op.
ANKI_USE_RESULT VkAttachmentStoreOp convertStoreOp(AttachmentStoreOperation ak);

/// Convert buffer usage bitmask.
ANKI_USE_RESULT VkBufferUsageFlags convertBufferUsageBit(BufferUsageBit usageMask);

ANKI_USE_RESULT VkImageType convertTextureType(TextureType ak);

ANKI_USE_RESULT VkImageViewType convertTextureViewType(TextureType ak);

ANKI_USE_RESULT VkImageUsageFlags convertTextureUsage(TextureUsageBit ak, const PixelFormat& format);
/// @}

} // end namespace anki
