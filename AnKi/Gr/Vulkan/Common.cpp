// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/Common.h>

#define VOLK_IMPLEMENTATION
#include <Volk/volk.h>

namespace anki {

VkCompareOp convertCompareOp(CompareOperation ak)
{
	VkCompareOp out = VK_COMPARE_OP_NEVER;
	switch(ak)
	{
	case CompareOperation::kAlways:
		out = VK_COMPARE_OP_ALWAYS;
		break;
	case CompareOperation::kLess:
		out = VK_COMPARE_OP_LESS;
		break;
	case CompareOperation::kEqual:
		out = VK_COMPARE_OP_EQUAL;
		break;
	case CompareOperation::kLessEqual:
		out = VK_COMPARE_OP_LESS_OR_EQUAL;
		break;
	case CompareOperation::kGreater:
		out = VK_COMPARE_OP_GREATER;
		break;
	case CompareOperation::kGreaterEqual:
		out = VK_COMPARE_OP_GREATER_OR_EQUAL;
		break;
	case CompareOperation::kNotEqual:
		out = VK_COMPARE_OP_NOT_EQUAL;
		break;
	case CompareOperation::kNever:
		out = VK_COMPARE_OP_NEVER;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkPrimitiveTopology convertTopology(PrimitiveTopology ak)
{
	VkPrimitiveTopology out = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	switch(ak)
	{
	case PrimitiveTopology::kPoints:
		out = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case PrimitiveTopology::kLines:
		out = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	case PrimitiveTopology::kLineStip:
		out = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		break;
	case PrimitiveTopology::kTriangles:
		out = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		break;
	case PrimitiveTopology::kTriangleStrip:
		out = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		break;
	case PrimitiveTopology::kPatchs:
		out = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkPolygonMode convertFillMode(FillMode ak)
{
	VkPolygonMode out = VK_POLYGON_MODE_FILL;
	switch(ak)
	{
	case FillMode::kPoints:
		out = VK_POLYGON_MODE_POINT;
		break;
	case FillMode::kWireframe:
		out = VK_POLYGON_MODE_LINE;
		break;
	case FillMode::kSolid:
		out = VK_POLYGON_MODE_FILL;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkCullModeFlags convertCullMode(FaceSelectionBit ak)
{
	VkCullModeFlags out = 0;
	switch(ak)
	{
	case FaceSelectionBit::kNone:
		out = VK_CULL_MODE_NONE;
		break;
	case FaceSelectionBit::kFront:
		out = VK_CULL_MODE_FRONT_BIT;
		break;
	case FaceSelectionBit::kBack:
		out = VK_CULL_MODE_BACK_BIT;
		break;
	case FaceSelectionBit::kFrontAndBack:
		out = VK_CULL_MODE_FRONT_BIT | VK_CULL_MODE_BACK_BIT;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkBlendFactor convertBlendFactor(BlendFactor ak)
{
	VkBlendFactor out = VK_BLEND_FACTOR_MAX_ENUM;
	switch(ak)
	{
	case BlendFactor::kZero:
		out = VK_BLEND_FACTOR_ZERO;
		break;
	case BlendFactor::kOne:
		out = VK_BLEND_FACTOR_ONE;
		break;
	case BlendFactor::kSrcColor:
		out = VK_BLEND_FACTOR_SRC_COLOR;
		break;
	case BlendFactor::kOneMinusSrcColor:
		out = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		break;
	case BlendFactor::kDstColor:
		out = VK_BLEND_FACTOR_DST_COLOR;
		break;
	case BlendFactor::kOneMinusDstColor:
		out = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		break;
	case BlendFactor::kSrcAlpha:
		out = VK_BLEND_FACTOR_SRC_ALPHA;
		break;
	case BlendFactor::kOneMinusSrcAlpha:
		out = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		break;
	case BlendFactor::kDstAlpha:
		out = VK_BLEND_FACTOR_DST_ALPHA;
		break;
	case BlendFactor::kOneMinusDstAlpha:
		out = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		break;
	case BlendFactor::kConstantColor:
		out = VK_BLEND_FACTOR_CONSTANT_COLOR;
		break;
	case BlendFactor::kOneMinusConstantColor:
		out = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		break;
	case BlendFactor::kConstantAlpha:
		out = VK_BLEND_FACTOR_CONSTANT_ALPHA;
		break;
	case BlendFactor::kOneMinusConstantAlpha:
		out = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		break;
	case BlendFactor::kSrcAlphaSaturate:
		out = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		break;
	case BlendFactor::kSrc1Color:
		out = VK_BLEND_FACTOR_SRC1_COLOR;
		break;
	case BlendFactor::kOneMinusSrc1Color:
		out = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		break;
	case BlendFactor::kSrc1Alpha:
		out = VK_BLEND_FACTOR_SRC1_ALPHA;
		break;
	case BlendFactor::kOneMinusSrc1Alpha:
		out = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkBlendOp convertBlendOperation(BlendOperation ak)
{
	VkBlendOp out = VK_BLEND_OP_MAX_ENUM;

	switch(ak)
	{
	case BlendOperation::kAdd:
		out = VK_BLEND_OP_ADD;
		break;
	case BlendOperation::kSubtract:
		out = VK_BLEND_OP_SUBTRACT;
		break;
	case BlendOperation::kReverseSubtract:
		out = VK_BLEND_OP_REVERSE_SUBTRACT;
		break;
	case BlendOperation::kMin:
		out = VK_BLEND_OP_MIN;
		break;
	case BlendOperation::kMax:
		out = VK_BLEND_OP_MAX;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkAttachmentLoadOp convertLoadOp(AttachmentLoadOperation ak)
{
	VkAttachmentLoadOp out = VK_ATTACHMENT_LOAD_OP_MAX_ENUM;

	switch(ak)
	{
	case AttachmentLoadOperation::kLoad:
		out = VK_ATTACHMENT_LOAD_OP_LOAD;
		break;
	case AttachmentLoadOperation::kClear:
		out = VK_ATTACHMENT_LOAD_OP_CLEAR;
		break;
	case AttachmentLoadOperation::kDontCare:
		out = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkAttachmentStoreOp convertStoreOp(AttachmentStoreOperation ak)
{
	VkAttachmentStoreOp out = VK_ATTACHMENT_STORE_OP_MAX_ENUM;

	switch(ak)
	{
	case AttachmentStoreOperation::kStore:
		out = VK_ATTACHMENT_STORE_OP_STORE;
		break;
	case AttachmentStoreOperation::kDontCare:
		out = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkBufferUsageFlags convertBufferUsageBit(BufferUsageBit usageMask)
{
	VkBufferUsageFlags out = 0;

	if(!!(usageMask & BufferUsageBit::kAllUniform))
	{
		out |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::kAllStorage))
	{
		out |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::kIndex))
	{
		out |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::kVertex))
	{
		out |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::kAllIndirect))
	{
		out |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::kTransferDestination))
	{
		out |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	if(!!(usageMask & BufferUsageBit::kTransferSource))
	{
		out |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}

	if(!!(usageMask & (BufferUsageBit::kAllTexture & BufferUsageBit::kAllRead)))
	{
		out |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	}

	if(!!(usageMask & (BufferUsageBit::kAllTexture & BufferUsageBit::kAllWrite)))
	{
		out |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::kAccelerationStructureBuild))
	{
		out |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	if(!!(usageMask & PrivateBufferUsageBit::kAccelerationStructureBuildScratch))
	{
		out |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // Spec says that this will be enough
	}

	if(!!(usageMask & PrivateBufferUsageBit::kAccelerationStructure))
	{
		out |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
	}

	if(!!(usageMask & BufferUsageBit::kSBT))
	{
		out |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
	}

	ANKI_ASSERT(out);

	return out;
}

VkImageType convertTextureType(TextureType ak)
{
	VkImageType out = VK_IMAGE_TYPE_MAX_ENUM;
	switch(ak)
	{
	case TextureType::kCube:
	case TextureType::kCubeArray:
	case TextureType::k2D:
	case TextureType::k2DArray:
		out = VK_IMAGE_TYPE_2D;
		break;
	case TextureType::k3D:
		out = VK_IMAGE_TYPE_3D;
		break;
	case TextureType::k1D:
		out = VK_IMAGE_TYPE_1D;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkImageViewType convertTextureViewType(TextureType ak)
{
	VkImageViewType out = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	switch(ak)
	{
	case TextureType::k1D:
		out = VK_IMAGE_VIEW_TYPE_1D;
		break;
	case TextureType::k2D:
		out = VK_IMAGE_VIEW_TYPE_2D;
		break;
	case TextureType::k2DArray:
		out = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	case TextureType::k3D:
		out = VK_IMAGE_VIEW_TYPE_3D;
		break;
	case TextureType::kCube:
		out = VK_IMAGE_VIEW_TYPE_CUBE;
		break;
	case TextureType::kCubeArray:
		out = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkImageUsageFlags convertTextureUsage(const TextureUsageBit ak, const Format format)
{
	VkImageUsageFlags out = 0;

	if(!!(ak & TextureUsageBit::kAllSampled))
	{
		out |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}

	if(!!(ak & TextureUsageBit::kAllImage))
	{
		out |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	if(!!(ak & (TextureUsageBit::kFramebufferRead | TextureUsageBit::kFramebufferWrite)))
	{
		if(getFormatInfo(format).isDepthStencil())
		{
			out |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else
		{
			out |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
	}

	if(!!(ak & TextureUsageBit::kFramebufferShadingRate))
	{
		out |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
	}

	if(!!(ak & TextureUsageBit::kTransferDestination))
	{
		out |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	if(!!(ak & TextureUsageBit::kGenerateMipmaps))
	{
		out |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	ANKI_ASSERT(out);
	return out;
}

VkStencilOp convertStencilOp(StencilOperation ak)
{
	VkStencilOp out = VK_STENCIL_OP_MAX_ENUM;

	switch(ak)
	{
	case StencilOperation::kKeep:
		out = VK_STENCIL_OP_KEEP;
		break;
	case StencilOperation::kZero:
		out = VK_STENCIL_OP_ZERO;
		break;
	case StencilOperation::kReplace:
		out = VK_STENCIL_OP_REPLACE;
		break;
	case StencilOperation::kIncrementAndClamp:
		out = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		break;
	case StencilOperation::kDecrementAndClamp:
		out = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		break;
	case StencilOperation::kInvert:
		out = VK_STENCIL_OP_INVERT;
		break;
	case StencilOperation::kIncrementAndWrap:
		out = VK_STENCIL_OP_INCREMENT_AND_WRAP;
		break;
	case StencilOperation::kDecrementAndWrap:
		out = VK_STENCIL_OP_DECREMENT_AND_WRAP;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

VkShaderStageFlags convertShaderTypeBit(ShaderTypeBit bit)
{
	ANKI_ASSERT(bit != ShaderTypeBit::kNone);

	VkShaderStageFlags out = 0;
	if(!!(bit & ShaderTypeBit::kVertex))
	{
		out |= VK_SHADER_STAGE_VERTEX_BIT;
	}

	if(!!(bit & ShaderTypeBit::kTessellationControl))
	{
		out |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	}

	if(!!(bit & ShaderTypeBit::kTessellationEvaluation))
	{
		out |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	}

	if(!!(bit & ShaderTypeBit::kGeometry))
	{
		out |= VK_SHADER_STAGE_GEOMETRY_BIT;
	}

	if(!!(bit & ShaderTypeBit::kFragment))
	{
		out |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	if(!!(bit & ShaderTypeBit::kCompute))
	{
		out |= VK_SHADER_STAGE_COMPUTE_BIT;
	}

	if(!!(bit & ShaderTypeBit::kRayGen))
	{
		out |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	}

	if(!!(bit & ShaderTypeBit::kAnyHit))
	{
		out |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	}

	if(!!(bit & ShaderTypeBit::kClosestHit))
	{
		out |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	}

	if(!!(bit & ShaderTypeBit::kMiss))
	{
		out |= VK_SHADER_STAGE_MISS_BIT_KHR;
	}

	if(!!(bit & ShaderTypeBit::kIntersection))
	{
		out |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
	}

	if(!!(bit & ShaderTypeBit::kCallable))
	{
		out |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
	}

	ANKI_ASSERT(out != 0);
	ANKI_ASSERT(__builtin_popcount(U32(bit)) == __builtin_popcount(out));
	return out;
}

const char* vkResultToString(VkResult res)
{
	const char* out;

	switch(res)
	{
	case VK_SUCCESS:
		out = "VK_SUCCESS";
		break;
	case VK_NOT_READY:
		out = "VK_NOT_READY";
		break;
	case VK_TIMEOUT:
		out = "VK_TIMEOUT";
		break;
	case VK_EVENT_SET:
		out = "VK_EVENT_SET";
		break;
	case VK_EVENT_RESET:
		out = "VK_EVENT_RESET";
		break;
	case VK_INCOMPLETE:
		out = "VK_INCOMPLETE";
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		out = "VK_ERROR_OUT_OF_HOST_MEMORY";
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		out = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		out = "VK_ERROR_INITIALIZATION_FAILED";
		break;
	case VK_ERROR_DEVICE_LOST:
		out = "VK_ERROR_DEVICE_LOST";
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		out = "VK_ERROR_MEMORY_MAP_FAILED";
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		out = "VK_ERROR_LAYER_NOT_PRESENT";
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		out = "VK_ERROR_EXTENSION_NOT_PRESENT";
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		out = "VK_ERROR_FEATURE_NOT_PRESENT";
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		out = "VK_ERROR_INCOMPATIBLE_DRIVER";
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		out = "VK_ERROR_TOO_MANY_OBJECTS";
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		out = "VK_ERROR_FORMAT_NOT_SUPPORTED";
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		out = "VK_ERROR_FRAGMENTED_POOL";
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		out = "VK_ERROR_OUT_OF_POOL_MEMORY";
		break;
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		out = "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		break;
	case VK_ERROR_SURFACE_LOST_KHR:
		out = "VK_ERROR_SURFACE_LOST_KHR";
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		out = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		break;
	case VK_SUBOPTIMAL_KHR:
		out = "VK_SUBOPTIMAL_KHR";
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		out = "VK_ERROR_OUT_OF_DATE_KHR";
		break;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		out = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		out = "VK_ERROR_VALIDATION_FAILED_EXT";
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		out = "VK_ERROR_INVALID_SHADER_NV";
		break;
	case VK_ERROR_FRAGMENTATION_EXT:
		out = "VK_ERROR_FRAGMENTATION_EXT";
		break;
	case VK_ERROR_NOT_PERMITTED_EXT:
		out = "VK_ERROR_NOT_PERMITTED_EXT";
		break;
	default:
		out = "Unknown VkResult";
		break;
	}

	return out;
}

} // end namespace anki
