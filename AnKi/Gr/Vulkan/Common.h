// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Common.h>

#if ANKI_WINDOWING_SYSTEM_HEADLESS
// Do nothing
#elif ANKI_OS_LINUX
#	define VK_USE_PLATFORM_XCB_KHR 1
#	define VK_USE_PLATFORM_XLIB_KHR 1
#elif ANKI_OS_WINDOWS
#	define VK_USE_PLATFORM_WIN32_KHR 1
#elif ANKI_OS_ANDROID
#	define VK_USE_PLATFORM_ANDROID_KHR 1
#else
#	error Not implemented
#endif
#include <Volk/volk.h>

// Cleanup global namespace from these dirty libaries
#if ANKI_OS_LINUX
#	undef Bool
#elif ANKI_OS_WINDOWS
#	include <AnKi/Util/CleanupWindows.h>
#endif

namespace anki {

// Forward
class GrManagerImpl;

/// @addtogroup vulkan
/// @{

#define ANKI_VK_LOGI(...) ANKI_LOG("VK", kNormal, __VA_ARGS__)
#define ANKI_VK_LOGE(...) ANKI_LOG("VK", kError, __VA_ARGS__)
#define ANKI_VK_LOGW(...) ANKI_LOG("VK", kWarning, __VA_ARGS__)
#define ANKI_VK_LOGF(...) ANKI_LOG("VK", kFatal, __VA_ARGS__)
#define ANKI_VK_LOGV(...) ANKI_LOG("VK", kVerbose, __VA_ARGS__)

#define ANKI_VK_SELF(class_) class_& self = *static_cast<class_*>(this)
#define ANKI_VK_SELF_CONST(class_) const class_& self = *static_cast<const class_*>(this)

enum class DescriptorType : U8
{
	kCombinedTextureSampler,
	kTexture,
	kSampler,
	kUniformBuffer,
	kStorageBuffer,
	kImage,
	kReadTextureBuffer,
	kReadWriteTextureBuffer,
	kAccelerationStructure,

	kCount
};

enum class VulkanExtensions : U32
{
	kNone = 0,
	kKHR_xcb_surface = 1 << 1,
	kKHR_xlib_surface = 1 << 2,
	kKHR_win32_surface = 1 << 3,
	kKHR_android_surface = 1 << 4,
	kEXT_headless_surface = 1 << 5,
	kKHR_swapchain = 1 << 6,
	kKHR_surface = 1 << 7,
	kEXT_debug_utils = 1 << 8,
	kAMD_shader_info = 1 << 9,
	kAMD_rasterization_order = 1 << 10,
	kKHR_ray_tracing = 1 << 11,
	kKHR_pipeline_executable_properties = 1 << 12,
	kEXT_descriptor_indexing = 1 << 13,
	kKHR_buffer_device_address = 1 << 14,
	kEXT_scalar_block_layout = 1 << 15,
	kKHR_timeline_semaphore = 1 << 16,
	kKHR_shader_float16_int8 = 1 << 17,
	kKHR_shader_atomic_int64 = 1 << 18,
	kKHR_spirv_1_4 = 1 << 19,
	kKHR_shader_float_controls = 1 << 20,
	kKHR_sampler_filter_min_max = 1 << 21,
	kKHR_create_renderpass_2 = 1 << 22,
	kKHR_fragment_shading_rate = 1 << 23,
	kEXT_astc_decode_mode = 1 << 24,
	kEXT_texture_compression_astc_hdr = 1 << 25,
	kNVX_binary_import = 1 << 26,
	kNVX_image_view_handle = 1 << 27,
	kKHR_push_descriptor = 1 << 28,
	kKHR_maintenance_4 = 1 << 29,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VulkanExtensions)

enum class VulkanQueueType : U8
{
	kGeneral,
	kCompute,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VulkanQueueType)

using VulkanQueueFamilies = Array<U32, U32(VulkanQueueType::kCount)>;

/// @name Constants
/// @{
constexpr U32 kDescriptorPoolInitialSize = 64;
constexpr F32 kDescriptorPoolSizeScale = 2.0f;
constexpr U32 kDescriptorBufferedFrameCount = 60 * 5; ///< How many frames worth of descriptors to buffer.

/// There is no need to ask for a fence or a semaphore to be waited for more than 10 seconds. The GPU will timeout
/// anyway.
constexpr Second kMaxFenceOrSemaphoreWaitTime = 10.0;
/// @}

/// Some internal buffer usage flags.
class PrivateBufferUsageBit
{
public:
	static constexpr BufferUsageBit kAccelerationStructureBuildScratch = BufferUsageBit(1ull << 29ull);
	static constexpr BufferUsageBit kAccelerationStructure = static_cast<BufferUsageBit>(1ull << 30ull);

	static constexpr BufferUsageBit kAllPrivate = kAccelerationStructureBuildScratch | kAccelerationStructure;
};
static_assert(!(BufferUsageBit::kAll & PrivateBufferUsageBit::kAllPrivate), "Update the bits in PrivateBufferUsageBit");

/// Check if a vulkan function failed. It will abort on failure.
#define ANKI_VK_CHECKF(x) \
	do \
	{ \
		VkResult rez; \
		if(ANKI_UNLIKELY((rez = (x)) < 0)) \
		{ \
			ANKI_VK_LOGF("Vulkan function failed (VkResult: %s): %s", vkResultToString(rez), #x); \
		} \
	} while(0)

/// Check if a vulkan function failed.
#define ANKI_VK_CHECK(x) \
	do \
	{ \
		VkResult rez; \
		if(ANKI_UNLIKELY((rez = (x)) < 0)) \
		{ \
			ANKI_VK_LOGE("Vulkan function failed (VkResult: %s): %s", vkResultToString(rez), #x); \
			return Error::kFunctionFailed; \
		} \
	} while(0)

/// Convert compare op.
[[nodiscard]] VkCompareOp convertCompareOp(CompareOperation ak);

/// Convert format.
[[nodiscard]] inline VkFormat convertFormat(const Format ak)
{
	ANKI_ASSERT(ak != Format::kNone);
	const VkFormat out = static_cast<VkFormat>(ak);
	return out;
}

/// Get format aspect mask.
[[nodiscard]] inline DepthStencilAspectBit getImageAspectFromFormat(const Format ak)
{
	DepthStencilAspectBit out = DepthStencilAspectBit::kNone;
	if(getFormatInfo(ak).isStencil())
	{
		out = DepthStencilAspectBit::kStencil;
	}

	if(getFormatInfo(ak).isDepth())
	{
		out |= DepthStencilAspectBit::kDepth;
	}

	return out;
}

/// Convert image aspect.
[[nodiscard]] inline VkImageAspectFlags convertImageAspect(const DepthStencilAspectBit ak)
{
	VkImageAspectFlags out = 0;
	if(!!(ak & DepthStencilAspectBit::kDepth))
	{
		out |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	if(!!(ak & DepthStencilAspectBit::kStencil))
	{
		out |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	if(!out)
	{
		out = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	return out;
}

/// Convert topology.
[[nodiscard]] VkPrimitiveTopology convertTopology(PrimitiveTopology ak);

/// Convert fill mode.
[[nodiscard]] VkPolygonMode convertFillMode(FillMode ak);

/// Convert cull mode.
[[nodiscard]] VkCullModeFlags convertCullMode(FaceSelectionBit ak);

/// Convert blend method.
[[nodiscard]] VkBlendFactor convertBlendFactor(BlendFactor ak);

/// Convert blend function.
[[nodiscard]] VkBlendOp convertBlendOperation(BlendOperation ak);

/// Convert color write mask.
[[nodiscard]] inline VkColorComponentFlags convertColorWriteMask(ColorBit ak)
{
	return static_cast<VkColorComponentFlags>(ak);
}

/// Convert load op.
[[nodiscard]] VkAttachmentLoadOp convertLoadOp(AttachmentLoadOperation ak);

/// Convert store op.
[[nodiscard]] VkAttachmentStoreOp convertStoreOp(AttachmentStoreOperation ak);

/// Convert buffer usage bitmask.
[[nodiscard]] VkBufferUsageFlags convertBufferUsageBit(BufferUsageBit usageMask);

[[nodiscard]] VkImageType convertTextureType(TextureType ak);

[[nodiscard]] VkImageViewType convertTextureViewType(TextureType ak);

[[nodiscard]] VkImageUsageFlags convertTextureUsage(const TextureUsageBit ak, const Format format);

[[nodiscard]] VkStencilOp convertStencilOp(StencilOperation ak);

[[nodiscard]] VkShaderStageFlags convertShaderTypeBit(ShaderTypeBit bit);

[[nodiscard]] inline VkVertexInputRate convertVertexStepRate(VertexStepRate ak)
{
	VkVertexInputRate out;
	switch(ak)
	{
	case VertexStepRate::kVertex:
		out = VK_VERTEX_INPUT_RATE_VERTEX;
		break;
	case VertexStepRate::kInstance:
		out = VK_VERTEX_INPUT_RATE_INSTANCE;
		break;
	default:
		ANKI_ASSERT(0);
		out = VK_VERTEX_INPUT_RATE_INSTANCE;
	}
	return out;
}

[[nodiscard]] inline VkDescriptorType convertDescriptorType(DescriptorType ak)
{
	VkDescriptorType out;
	switch(ak)
	{
	case DescriptorType::kCombinedTextureSampler:
		out = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;
	case DescriptorType::kTexture:
		out = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		break;
	case DescriptorType::kSampler:
		out = VK_DESCRIPTOR_TYPE_SAMPLER;
		break;
	case DescriptorType::kUniformBuffer:
		out = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		break;
	case DescriptorType::kReadTextureBuffer:
		out = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		break;
	case DescriptorType::kReadWriteTextureBuffer:
		out = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		break;
	case DescriptorType::kStorageBuffer:
		out = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		break;
	case DescriptorType::kImage:
		out = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		break;
	case DescriptorType::kAccelerationStructure:
		out = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		break;
	default:
		out = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		ANKI_ASSERT(0);
	}

	return out;
}

[[nodiscard]] inline VkIndexType convertIndexType(IndexType ak)
{
	VkIndexType out;
	switch(ak)
	{
	case IndexType::kU16:
		out = VK_INDEX_TYPE_UINT16;
		break;
	case IndexType::kU32:
		out = VK_INDEX_TYPE_UINT32;
		break;
	default:
		ANKI_ASSERT(0);
		out = VK_INDEX_TYPE_MAX_ENUM;
	}

	return out;
}

[[nodiscard]] inline VkRasterizationOrderAMD convertRasterizationOrder(RasterizationOrder ak)
{
	VkRasterizationOrderAMD out;
	switch(ak)
	{
	case RasterizationOrder::kOrdered:
		out = VK_RASTERIZATION_ORDER_STRICT_AMD;
		break;
	case RasterizationOrder::kRelaxed:
		out = VK_RASTERIZATION_ORDER_RELAXED_AMD;
		break;
	default:
		ANKI_ASSERT(0);
		out = VK_RASTERIZATION_ORDER_STRICT_AMD;
	}

	return out;
}

[[nodiscard]] inline VkAccelerationStructureTypeKHR convertAccelerationStructureType(AccelerationStructureType ak)
{
	VkAccelerationStructureTypeKHR out;
	switch(ak)
	{
	case AccelerationStructureType::kBottomLevel:
		out = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		break;
	case AccelerationStructureType::kTopLevel:
		out = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		break;
	default:
		ANKI_ASSERT(0);
		out = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	}

	return out;
}

[[nodiscard]] const char* vkResultToString(VkResult res);

[[nodiscard]] inline VkExtent2D convertVrsShadingRate(VrsRate rate)
{
	VkExtent2D out = {};
	switch(rate)
	{
	case VrsRate::k1x1:
		out = {1, 1};
		break;
	case VrsRate::k2x1:
		out = {2, 1};
		break;
	case VrsRate::k1x2:
		out = {1, 2};
		break;
	case VrsRate::k2x2:
		out = {2, 2};
		break;
	case VrsRate::k4x2:
		out = {4, 2};
		break;
	case VrsRate::k2x4:
		out = {2, 4};
		break;
	case VrsRate::k4x4:
		out = {4, 4};
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}
/// @}

} // end namespace anki
