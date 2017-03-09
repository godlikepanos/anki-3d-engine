// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

/// @addtogrou/cygdrive/c/VulkanSDK/1.0.39.1/Include/vulkan/vulkan.h
/// @{

#define ANKI_VK_LOGI(...) ANKI_LOG("VK  ", NORMAL, __VA_ARGS__)
#define ANKI_VK_LOGE(...) ANKI_LOG("VK  ", ERROR, __VA_ARGS__)
#define ANKI_VK_LOGW(...) ANKI_LOG("VK  ", WARNING, __VA_ARGS__)
#define ANKI_VK_LOGF(...) ANKI_LOG("VK  ", FATAL, __VA_ARGS__)

enum class VulkanExtensions : U8
{
	NONE = 0,
	KHR_MAINENANCE1 = 1 << 0,
	AMD_NEGATIVE_VIEWPORT_HEIGHT = 1 << 1,
	KHR_XCB_SURFACE = 1 << 2,
	KHR_WIN32_SURFACE = 1 << 3,
	KHR_SWAPCHAIN = 1 << 4,
	KHR_SURFACE = 1 << 5,
	EXT_DEBUG_MARKER = 1 << 6,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VulkanExtensions, inline)

/// @name Constants
/// @{
const U DESCRIPTOR_POOL_INITIAL_SIZE = 64;
const F32 DESCRIPTOR_POOL_SIZE_SCALE = 2.0;
const U DESCRIPTOR_FRAME_BUFFERING = 60 * 5; ///< How many frames worth of descriptors to buffer.
/// @}

/// Check if a vulkan function failed. It will abort on failure.
#define ANKI_VK_CHECKF(x)                                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		VkResult rez;                                                                                                  \
		if((rez = (x)) < 0)                                                                                            \
		{                                                                                                              \
			ANKI_VK_LOGF("Vulkan function failed (VkResult: %d): %s", rez, #x);                                        \
		}                                                                                                              \
	} while(0)

/// Check if a vulkan function failed.
#define ANKI_VK_CHECK(x)                                                                                               \
	do                                                                                                                 \
	{                                                                                                                  \
		VkResult rez;                                                                                                  \
		if((rez = (x)) < 0)                                                                                            \
		{                                                                                                              \
			ANKI_VK_LOGE("Vulkan function failed (VkResult: %d): %s", rez, #x);                                        \
			return ErrorCode::FUNCTION_FAILED;                                                                         \
		}                                                                                                              \
	} while(0)

/// Convert compare op.
ANKI_USE_RESULT VkCompareOp convertCompareOp(CompareOperation ak);

/// Convert format.
ANKI_USE_RESULT VkFormat convertFormat(PixelFormat ak);

/// Get format aspect mask.
ANKI_USE_RESULT VkImageAspectFlags convertImageAspect(PixelFormat ak);

ANKI_USE_RESULT inline Bool formatIsDepthStencil(PixelFormat fmt)
{
	VkImageAspectFlags aspect = convertImageAspect(fmt);
	return !!(aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
}

/// Convert topology.
ANKI_USE_RESULT VkPrimitiveTopology convertTopology(PrimitiveTopology ak);

/// Convert fill mode.
ANKI_USE_RESULT VkPolygonMode convertFillMode(FillMode ak);

/// Convert cull mode.
ANKI_USE_RESULT VkCullModeFlags convertCullMode(FaceSelectionBit ak);

/// Convert blend method.
ANKI_USE_RESULT VkBlendFactor convertBlendFactor(BlendFactor ak);

/// Convert blend function.
ANKI_USE_RESULT VkBlendOp convertBlendOperation(BlendOperation ak);

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

ANKI_USE_RESULT VkStencilOp convertStencilOp(StencilOperation ak);

ANKI_USE_RESULT inline VkShaderStageFlagBits convertShaderTypeBit(ShaderTypeBit bit)
{
	ANKI_ASSERT(bit != ShaderTypeBit::NONE);
	return static_cast<VkShaderStageFlagBits>(bit);
}

ANKI_USE_RESULT inline VkVertexInputRate convertVertexStepRate(VertexStepRate ak)
{
	VkVertexInputRate out;
	switch(ak)
	{
	case VertexStepRate::VERTEX:
		out = VK_VERTEX_INPUT_RATE_VERTEX;
		break;
	case VertexStepRate::INSTANCE:
		out = VK_VERTEX_INPUT_RATE_INSTANCE;
		break;
	default:
		ANKI_ASSERT(0);
		out = VK_VERTEX_INPUT_RATE_INSTANCE;
	}
	return out;
}

ANKI_USE_RESULT inline VkDescriptorType convertDescriptorType(DescriptorType ak)
{
	VkDescriptorType out;
	switch(ak)
	{
	case DescriptorType::TEXTURE:
		out = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;
	case DescriptorType::UNIFORM_BUFFER:
		out = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		break;
	case DescriptorType::STORAGE_BUFFER:
		out = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		break;
	case DescriptorType::IMAGE:
		out = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		break;
	default:
		out = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		ANKI_ASSERT(0);
	}

	return out;
}

ANKI_USE_RESULT inline VkIndexType convertIndexType(IndexType ak)
{
	VkIndexType out;
	switch(ak)
	{
	case IndexType::U16:
		out = VK_INDEX_TYPE_UINT16;
		break;
	case IndexType::U32:
		out = VK_INDEX_TYPE_UINT32;
		break;
	default:
		ANKI_ASSERT(0);
		out = VK_INDEX_TYPE_MAX_ENUM;
	}

	return out;
}
/// @}

} // end namespace anki
