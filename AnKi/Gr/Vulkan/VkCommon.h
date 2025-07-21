// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Common.h>
#include <AnKi/Gr/BackendCommon/Common.h>

#if ANKI_WINDOWING_SYSTEM_HEADLESS
// Do nothing
#elif ANKI_OS_LINUX
#	define VK_USE_PLATFORM_WAYLAND_KHR 1
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

ANKI_PURE GrManagerImpl& getGrManagerImpl();
ANKI_PURE VkDevice getVkDevice();

enum class VulkanExtensions : U32
{
	kNone = 0,
	kKHR_wayland_surface = 1u << 1u,
	kKHR_win32_surface = 1u << 2u,
	kKHR_android_surface = 1u << 3u,
	kEXT_headless_surface = 1u << 4u,
	kKHR_swapchain = 1u << 5u,
	kKHR_surface = 1u << 6u,
	kEXT_debug_utils = 1u << 7u,
	kKHR_ray_tracing = 1u << 8u,
	kKHR_pipeline_executable_properties = 1u << 9u,
	kKHR_fragment_shading_rate = 1u << 10u,
	kEXT_astc_decode_mode = 1u << 11u,
	kEXT_texture_compression_astc_hdr = 1u << 12u,
	kNVX_binary_import = 1u << 13u,
	kNVX_image_view_handle = 1u << 14u,
	kKHR_push_descriptor = 1u << 15u,
	kEXT_mesh_shader = 1u << 16u,
	kKHR_fragment_shader_barycentric = 1u << 17u,
	kKHR_ray_tracing_position_fetch = 1u << 18u,
	kKHR_xcb_surface = 1u << 19u,
	kKHR_xlib_surface = 1u << 20u,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VulkanExtensions)

using VulkanQueueFamilies = Array<U32, U32(GpuQueueType::kCount)>;

/// @name Constants
/// @{
constexpr U32 kDescriptorPoolInitialSize = 64;
constexpr F32 kDescriptorPoolSizeScale = 2.0f;
constexpr U32 kDescriptorBufferedFrameCount = 60 * 5; ///< How many frames worth of descriptors to buffer.
/// @}

/// Some internal buffer usage flags.
class PrivateBufferUsageBit
{
public:
	/// Buffer that holds the memory for the actual AS.
	static constexpr BufferUsageBit kAccelerationStructure = BufferUsageBit(1ull << 30ull);

	static constexpr BufferUsageBit kAllPrivate = kAccelerationStructure;
};
static_assert(!(BufferUsageBit::kAll & PrivateBufferUsageBit::kAllPrivate), "Update the bits in PrivateBufferUsageBit");

/// Check if a vulkan function failed. It will abort on failure.
#define ANKI_VK_CHECKF(x) \
	do \
	{ \
		VkResult rez; \
		if((rez = (x)) < 0) [[unlikely]] \
		{ \
			ANKI_VK_LOGF("Vulkan function failed (VkResult: %s): %s", vkResultToString(rez), #x); \
		} \
	} while(0)

/// Check if a vulkan function failed.
#define ANKI_VK_CHECK(x) \
	do \
	{ \
		VkResult rez; \
		if((rez = (x)) < 0) [[unlikely]] \
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
[[nodiscard]] VkAttachmentLoadOp convertLoadOp(RenderTargetLoadOperation ak);

/// Convert store op.
[[nodiscard]] VkAttachmentStoreOp convertStoreOp(RenderTargetStoreOperation ak);

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
	case DescriptorType::kConstantBuffer:
		out = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case DescriptorType::kSrvStructuredBuffer:
	case DescriptorType::kUavStructuredBuffer:
	case DescriptorType::kSrvByteAddressBuffer:
	case DescriptorType::kUavByteAddressBuffer:
		out = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		break;
	case DescriptorType::kSrvTexelBuffer:
		out = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		break;
	case DescriptorType::kUavTexelBuffer:
		out = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		break;
	case DescriptorType::kSrvTexture:
		out = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		break;
	case DescriptorType::kUavTexture:
		out = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		break;
	case DescriptorType::kAccelerationStructure:
		out = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		break;
	case DescriptorType::kSampler:
		out = VK_DESCRIPTOR_TYPE_SAMPLER;
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

template<typename TMain, typename TPnext>
void appendPNextList(TMain& struc, TPnext* pNext)
{
	pNext->pNext = const_cast<void*>(struc.pNext);
	struc.pNext = pNext;
}
/// @}

} // end namespace anki
