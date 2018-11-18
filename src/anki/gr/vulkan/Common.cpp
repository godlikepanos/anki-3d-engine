// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/Common.h>

namespace anki
{

VkCompareOp convertCompareOp(CompareOperation ak)
{
	VkCompareOp out = VK_COMPARE_OP_NEVER;
	switch(ak)
	{
	case CompareOperation::ALWAYS:
		out = VK_COMPARE_OP_ALWAYS;
		break;
	case CompareOperation::LESS:
		out = VK_COMPARE_OP_LESS;
		break;
	case CompareOperation::EQUAL:
		out = VK_COMPARE_OP_EQUAL;
		break;
	case CompareOperation::LESS_EQUAL:
		out = VK_COMPARE_OP_LESS_OR_EQUAL;
		break;
	case CompareOperation::GREATER:
		out = VK_COMPARE_OP_GREATER;
		break;
	case CompareOperation::GREATER_EQUAL:
		out = VK_COMPARE_OP_GREATER_OR_EQUAL;
		break;
	case CompareOperation::NOT_EQUAL:
		out = VK_COMPARE_OP_NOT_EQUAL;
		break;
	case CompareOperation::NEVER:
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
	case POINTS:
		out = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case LINES:
		out = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	case LINE_STIP:
		out = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		break;
	case TRIANGLES:
		out = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		break;
	case TRIANGLE_STRIP:
		out = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		break;
	case PATCHES:
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
	case FillMode::POINTS:
		out = VK_POLYGON_MODE_POINT;
		break;
	case FillMode::WIREFRAME:
		out = VK_POLYGON_MODE_LINE;
		break;
	case FillMode::SOLID:
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
	case FaceSelectionBit::FRONT:
		out = VK_CULL_MODE_FRONT_BIT;
		break;
	case FaceSelectionBit::BACK:
		out = VK_CULL_MODE_BACK_BIT;
		break;
	case FaceSelectionBit::FRONT_AND_BACK:
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
	case BlendFactor::ZERO:
		out = VK_BLEND_FACTOR_ZERO;
		break;
	case BlendFactor::ONE:
		out = VK_BLEND_FACTOR_ONE;
		break;
	case BlendFactor::SRC_COLOR:
		out = VK_BLEND_FACTOR_SRC_COLOR;
		break;
	case BlendFactor::ONE_MINUS_SRC_COLOR:
		out = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		break;
	case BlendFactor::DST_COLOR:
		out = VK_BLEND_FACTOR_DST_COLOR;
		break;
	case BlendFactor::ONE_MINUS_DST_COLOR:
		out = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		break;
	case BlendFactor::SRC_ALPHA:
		out = VK_BLEND_FACTOR_SRC_ALPHA;
		break;
	case BlendFactor::ONE_MINUS_SRC_ALPHA:
		out = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		break;
	case BlendFactor::DST_ALPHA:
		out = VK_BLEND_FACTOR_DST_ALPHA;
		break;
	case BlendFactor::ONE_MINUS_DST_ALPHA:
		out = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		break;
	case BlendFactor::CONSTANT_COLOR:
		out = VK_BLEND_FACTOR_CONSTANT_COLOR;
		break;
	case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
		out = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		break;
	case BlendFactor::CONSTANT_ALPHA:
		out = VK_BLEND_FACTOR_CONSTANT_ALPHA;
		break;
	case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
		out = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		break;
	case BlendFactor::SRC_ALPHA_SATURATE:
		out = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		break;
	case BlendFactor::SRC1_COLOR:
		out = VK_BLEND_FACTOR_SRC1_COLOR;
		break;
	case BlendFactor::ONE_MINUS_SRC1_COLOR:
		out = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		break;
	case BlendFactor::SRC1_ALPHA:
		out = VK_BLEND_FACTOR_SRC1_ALPHA;
		break;
	case BlendFactor::ONE_MINUS_SRC1_ALPHA:
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
	case BlendOperation::ADD:
		out = VK_BLEND_OP_ADD;
		break;
	case BlendOperation::SUBTRACT:
		out = VK_BLEND_OP_SUBTRACT;
		break;
	case BlendOperation::REVERSE_SUBTRACT:
		out = VK_BLEND_OP_REVERSE_SUBTRACT;
		break;
	case BlendOperation::MIN:
		out = VK_BLEND_OP_MIN;
		break;
	case BlendOperation::MAX:
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
	case AttachmentLoadOperation::LOAD:
		out = VK_ATTACHMENT_LOAD_OP_LOAD;
		break;
	case AttachmentLoadOperation::CLEAR:
		out = VK_ATTACHMENT_LOAD_OP_CLEAR;
		break;
	case AttachmentLoadOperation::DONT_CARE:
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
	case AttachmentStoreOperation::STORE:
		out = VK_ATTACHMENT_STORE_OP_STORE;
		break;
	case AttachmentStoreOperation::DONT_CARE:
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

	if(!!(usageMask & BufferUsageBit::UNIFORM_ALL))
	{
		out |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::STORAGE_ALL))
	{
		out |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::INDEX))
	{
		out |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::VERTEX))
	{
		out |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	if(!!(usageMask & BufferUsageBit::INDIRECT_ALL))
	{
		out |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}

	if(!!(usageMask
		   & (BufferUsageBit::BUFFER_UPLOAD_DESTINATION | BufferUsageBit::FILL | BufferUsageBit::QUERY_RESULT)))
	{
		out |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	if(!!(usageMask & (BufferUsageBit::BUFFER_UPLOAD_SOURCE | BufferUsageBit::TEXTURE_UPLOAD_SOURCE)))
	{
		out |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}

	if(!!(usageMask & BufferUsageBit::TEXTURE_ALL))
	{
		out |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	}

	ANKI_ASSERT(out);

	return out;
}

VkImageType convertTextureType(TextureType ak)
{
	VkImageType out = VK_IMAGE_TYPE_MAX_ENUM;
	switch(ak)
	{
	case TextureType::CUBE:
	case TextureType::CUBE_ARRAY:
	case TextureType::_2D:
	case TextureType::_2D_ARRAY:
		out = VK_IMAGE_TYPE_2D;
		break;
	case TextureType::_3D:
		out = VK_IMAGE_TYPE_3D;
		break;
	case TextureType::_1D:
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
	case TextureType::_1D:
		out = VK_IMAGE_VIEW_TYPE_1D;
		break;
	case TextureType::_2D:
		out = VK_IMAGE_VIEW_TYPE_2D;
		break;
	case TextureType::_2D_ARRAY:
		out = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	case TextureType::_3D:
		out = VK_IMAGE_VIEW_TYPE_3D;
		break;
	case TextureType::CUBE:
		out = VK_IMAGE_VIEW_TYPE_CUBE;
		break;
	case TextureType::CUBE_ARRAY:
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

	if(!!(ak & TextureUsageBit::SAMPLED_ALL))
	{
		out |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}

	if(!!(ak & TextureUsageBit::IMAGE_ALL))
	{
		out |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	if(!!(ak & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE))
	{
		if(formatIsDepthStencil(format))
		{
			out |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else
		{
			out |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
	}

	if(!!(ak & TextureUsageBit::GENERATE_MIPMAPS))
	{
		out |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	if(!!(ak & TextureUsageBit::TRANSFER_DESTINATION))
	{
		out |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	if(!!(ak & TextureUsageBit::CLEAR))
	{
		out |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	ANKI_ASSERT(out);
	return out;
}

VkStencilOp convertStencilOp(StencilOperation ak)
{
	VkStencilOp out = VK_STENCIL_OP_MAX_ENUM;

	switch(ak)
	{
	case StencilOperation::KEEP:
		out = VK_STENCIL_OP_KEEP;
		break;
	case StencilOperation::ZERO:
		out = VK_STENCIL_OP_ZERO;
		break;
	case StencilOperation::REPLACE:
		out = VK_STENCIL_OP_REPLACE;
		break;
	case StencilOperation::INCREMENT_AND_CLAMP:
		out = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		break;
	case StencilOperation::DECREMENT_AND_CLAMP:
		out = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		break;
	case StencilOperation::INVERT:
		out = VK_STENCIL_OP_INVERT;
		break;
	case StencilOperation::INCREMENT_AND_WRAP:
		out = VK_STENCIL_OP_INCREMENT_AND_WRAP;
		break;
	case StencilOperation::DECREMENT_AND_WRAP:
		out = VK_STENCIL_OP_DECREMENT_AND_WRAP;
		break;
	default:
		ANKI_ASSERT(0);
	}

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
