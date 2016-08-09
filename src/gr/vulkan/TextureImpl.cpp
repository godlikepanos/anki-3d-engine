// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/common/Misc.h>

namespace anki
{

//==============================================================================
// TextureImpl::CreateContext                                                  =
//==============================================================================

class TextureImpl::CreateContext
{
public:
	TextureInitInfo m_init;
	VkImageCreateInfo m_imgCi = {};
};

//==============================================================================
// TextureImpl                                                                 =
//==============================================================================

//==============================================================================
TextureImpl::TextureImpl(GrManager* manager)
	: VulkanObject(manager)
{
}

//==============================================================================
TextureImpl::~TextureImpl()
{
	if(m_viewHandle)
	{
		vkDestroyImageView(getDevice(), m_viewHandle, nullptr);
	}

	for(VkImageView view : m_viewsEveryLevel)
	{
		vkDestroyImageView(getDevice(), view, nullptr);
	}

	m_viewsEveryLevel.destroy(getAllocator());

	if(m_imageHandle)
	{
		vkDestroyImage(getDevice(), m_imageHandle, nullptr);
	}

	if(m_memHandle)
	{
		getGrManagerImpl().freeMemory(m_memIdx, m_memHandle);
	}
}

//==============================================================================
VkFormatFeatureFlags TextureImpl::calcFeatures(const TextureInitInfo& init)
{
	VkFormatFeatureFlags flags = 0;

	if(init.m_mipmapsCount > 1
		&& !!(init.m_usage & TextureUsageBit::GENERATE_MIPMAPS))
	{
		// May be used for mip gen.
		flags |=
			VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT;
	}

	if(!!(init.m_usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE))
	{
		if(formatIsDepthStencil(init.m_format))
		{
			flags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else
		{
			flags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
				| VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
		}
	}

	if(!!(init.m_usage & TextureUsageBit::SAMPLED_ALL))
	{
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	}

	ANKI_ASSERT(flags);
	return flags;
}

//==============================================================================
VkImageCreateFlags TextureImpl::calcCreateFlags(const TextureInitInfo& init)
{
	VkImageCreateFlags flags = 0;
	if(init.m_type == TextureType::CUBE
		|| init.m_type == TextureType::CUBE_ARRAY)
	{
		flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	return flags;
}

//==============================================================================
Bool TextureImpl::imageSupported(const TextureInitInfo& init)
{
	VkImageFormatProperties props = {};

	VkResult res = vkGetPhysicalDeviceImageFormatProperties(
		getGrManagerImpl().getPhysicalDevice(),
		convertFormat(init.m_format),
		convertTextureType(init.m_type),
		VK_IMAGE_TILING_OPTIMAL,
		convertTextureUsage(init.m_usage, init.m_format),
		calcCreateFlags(init),
		&props);

	if(res == VK_ERROR_FORMAT_NOT_SUPPORTED)
	{
		return false;
	}
	else
	{
		ANKI_ASSERT(res == VK_SUCCESS);
		return true;
	}
}

//==============================================================================
Error TextureImpl::init(const TextureInitInfo& init_, Texture* tex)
{
	TextureInitInfo init = init_;
	ANKI_ASSERT(textureInitInfoValid(init));
	m_sampler = getGrManager().newInstanceCached<Sampler>(init.m_sampling);

	// Set some stuff
	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_type = init.m_type;

	if(m_type == TextureType::_3D)
	{
		m_mipCount = min<U>(init.m_mipmapsCount,
			computeMaxMipmapCount3d(m_width, m_height, m_depth));
	}
	else
	{
		m_mipCount = min<U>(
			init.m_mipmapsCount, computeMaxMipmapCount2d(m_width, m_height));
	}
	init.m_mipmapsCount = m_mipCount;

	m_layerCount = init.m_layerCount;

	m_format = init.m_format;
	m_depthStencil = formatIsDepthStencil(m_format);
	if(m_depthStencil)
	{
		m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		m_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	m_usage = init.m_usage;

	CreateContext ctx;
	ctx.m_init = init;
	ANKI_CHECK(initImage(ctx));
	ANKI_CHECK(initView(ctx));

	// Transition the image layout from undefined to something relevant
	if(!!init.m_initialUsage)
	{
		ANKI_ASSERT(usageValid(init.m_initialUsage));
		ANKI_ASSERT(!(init.m_initialUsage & TextureUsageBit::GENERATE_MIPMAPS)
			&& "That doesn't make any sense");

		VkImageLayout initialLayout = computeLayout(init.m_initialUsage, 0);

		CommandBufferInitInfo cmdbinit;
		cmdbinit.m_flags =
			CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SMALL_BATCH;
		CommandBufferPtr cmdb =
			getGrManager().newInstance<CommandBuffer>(cmdbinit);

		VkImageSubresourceRange range;
		range.aspectMask = m_aspect;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = m_layerCount;
		range.levelCount = m_mipCount;

		cmdb->getImplementation().setImageBarrier(
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			initialLayout,
			TexturePtr(tex),
			range);

		cmdb->getImplementation().endRecording();
		getGrManagerImpl().flushCommandBuffer(cmdb);
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error TextureImpl::initImage(CreateContext& ctx)
{
	TextureInitInfo& init = ctx.m_init;

	// Check if format is supported
	Bool supported;
	while(!(supported = imageSupported(init)))
	{
		// Try to find a fallback
		if(init.m_format.m_components == ComponentFormat::R8G8B8)
		{
			ANKI_ASSERT((init.m_usage & TextureUsageBit::UPLOAD)
					== TextureUsageBit::NONE
				&& "Can't do that ATM");
			init.m_format.m_components = ComponentFormat::R8G8B8A8;
			m_format = init.m_format;
		}
		else
		{
			break;
		}
	}

	if(!supported)
	{
		ANKI_LOGE("Unsupported texture format: %u %u",
			U(init.m_format.m_components),
			U(init.m_format.m_transform));
	}

	// Contunue with the creation
	m_type = init.m_type;

	VkImageCreateInfo& ci = ctx.m_imgCi;
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.flags = calcCreateFlags(init);
	ci.imageType = convertTextureType(init.m_type);
	ci.format = convertFormat(init.m_format);
	ci.extent.width = init.m_width;
	ci.extent.height = init.m_height;

	switch(init.m_type)
	{
	case TextureType::_1D:
	case TextureType::_2D:
		ci.extent.depth = 1;
		ci.arrayLayers = 1;
		break;
	case TextureType::_2D_ARRAY:
		ci.extent.depth = 1;
		ci.arrayLayers = init.m_layerCount;
		break;
	case TextureType::CUBE:
		ci.extent.depth = 1;
		ci.arrayLayers = 6;
		break;
	case TextureType::CUBE_ARRAY:
		ci.extent.depth = 1;
		ci.arrayLayers = 6 * init.m_layerCount;
		break;
	case TextureType::_3D:
		ci.extent.depth = init.m_depth;
		ci.arrayLayers = 1;
		break;
	default:
		ANKI_ASSERT(0);
	}

	ci.mipLevels = m_mipCount;

	ci.samples = VK_SAMPLE_COUNT_1_BIT;
	ci.tiling = VK_IMAGE_TILING_OPTIMAL;
	ci.usage = convertTextureUsage(init.m_usage, init.m_format);
	ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.queueFamilyIndexCount = 1;
	U32 queueIdx = getGrManagerImpl().getGraphicsQueueFamily();
	ci.pQueueFamilyIndices = &queueIdx;
	ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	ANKI_VK_CHECK(vkCreateImage(getDevice(), &ci, nullptr, &m_imageHandle));

	// Allocate memory
	//
	VkMemoryRequirements req = {};
	vkGetImageMemoryRequirements(getDevice(), m_imageHandle, &req);

	m_memIdx = getGrManagerImpl().findMemoryType(req.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Fallback
	if(m_memIdx == MAX_U32)
	{
		m_memIdx = getGrManagerImpl().findMemoryType(
			req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
	}

	ANKI_ASSERT(m_memIdx != MAX_U32);

	// Allocate
	getGrManagerImpl().allocateMemory(
		m_memIdx, req.size, req.alignment, m_memHandle);

	// Bind mem to image
	ANKI_VK_CHECK(vkBindImageMemory(getDevice(),
		m_imageHandle,
		m_memHandle.m_memory,
		m_memHandle.m_offset));

	return ErrorCode::NONE;
}

//==============================================================================
Error TextureImpl::initView(CreateContext& ctx)
{
	const TextureInitInfo& init = ctx.m_init;

	VkImageViewCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ci.image = m_imageHandle;
	ci.viewType = convertTextureViewType(init.m_type);
	ci.format = ctx.m_imgCi.format;
	ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.subresourceRange.aspectMask = m_aspect;
	ci.subresourceRange.baseArrayLayer = 0;
	ci.subresourceRange.baseMipLevel = 0;
	ci.subresourceRange.layerCount = ctx.m_imgCi.arrayLayers;
	ci.subresourceRange.levelCount = ctx.m_imgCi.mipLevels;

	ANKI_VK_CHECK(vkCreateImageView(getDevice(), &ci, nullptr, &m_viewHandle));

	// Create the rest of the views
	if(!!(m_usage & TextureUsageBit::IMAGE_COMPUTE_READ_WRITE))
	{
		ci.subresourceRange.levelCount = 1;

		m_viewsEveryLevel.create(getAllocator(), m_mipCount - 1);

		for(U i = 0; i < m_viewsEveryLevel.getSize(); ++i)
		{
			ci.subresourceRange.baseMipLevel = i + 1;
			ANKI_VK_CHECK(vkCreateImageView(
				getDevice(), &ci, nullptr, &m_viewsEveryLevel[i]));
		}
	}

	return ErrorCode::NONE;
}

//==============================================================================
void TextureImpl::computeBarrierInfo(TextureUsageBit before,
	TextureUsageBit after,
	U level,
	VkPipelineStageFlags& srcStages,
	VkAccessFlags& srcAccesses,
	VkPipelineStageFlags& dstStages,
	VkAccessFlags& dstAccesses) const
{
	ANKI_ASSERT(level < m_mipCount);
	ANKI_ASSERT(usageValid(before) && usageValid(after));
	srcStages = 0;
	srcAccesses = 0;
	dstStages = 0;
	dstAccesses = 0;
	Bool lastLevel = level == m_mipCount - 1u;

	//
	// Before
	//
	if(!!(before & TextureUsageBit::SAMPLED_VERTEX))
	{
		srcStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		srcAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(before & TextureUsageBit::SAMPLED_TESSELLATION_CONTROL))
	{
		srcStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
		srcAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(before & TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION))
	{
		srcStages |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
		srcAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(before & TextureUsageBit::SAMPLED_GEOMETRY))
	{
		srcStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		srcAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(before & TextureUsageBit::SAMPLED_FRAGMENT))
	{
		srcStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		srcAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(before & TextureUsageBit::SAMPLED_COMPUTE)
		|| !!(before & TextureUsageBit::IMAGE_COMPUTE_READ))
	{
		srcStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		srcAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(before & TextureUsageBit::IMAGE_COMPUTE_WRITE))
	{
		srcStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		srcAccesses |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(!!(before & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ))
	{
		if(m_depthStencil)
		{
			srcStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
				| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			srcAccesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}
		else
		{
			srcStages |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			srcAccesses |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
	}

	if(!!(before & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE))
	{
		srcStages |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

		if(m_depthStencil)
		{
			srcAccesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else
		{
			srcAccesses |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
	}

	if(!!(before & TextureUsageBit::GENERATE_MIPMAPS))
	{
		srcStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;

		if(!lastLevel)
		{
			srcAccesses |= VK_ACCESS_TRANSFER_READ_BIT;
		}
		else
		{
			srcAccesses |= VK_ACCESS_TRANSFER_WRITE_BIT;
		}
	}

	if(!!(before & TextureUsageBit::UPLOAD))
	{
		srcStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		srcAccesses |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(!!(before & TextureUsageBit::CLEAR))
	{
		srcStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		srcAccesses |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(srcStages == 0)
	{
		srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	//
	// After
	//
	if(!!(after & TextureUsageBit::SAMPLED_VERTEX))
	{
		dstStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		dstAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(after & TextureUsageBit::SAMPLED_TESSELLATION_CONTROL))
	{
		dstStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
		dstAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(after & TextureUsageBit::SAMPLED_TESSELLATION_EVALUATION))
	{
		dstStages |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
		dstAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(after & TextureUsageBit::SAMPLED_GEOMETRY))
	{
		dstStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		dstAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(after & TextureUsageBit::SAMPLED_FRAGMENT))
	{
		dstStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dstAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(after & TextureUsageBit::SAMPLED_COMPUTE)
		|| !!(after & TextureUsageBit::IMAGE_COMPUTE_READ))
	{
		dstStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstAccesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(after & TextureUsageBit::IMAGE_COMPUTE_WRITE))
	{
		dstStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstAccesses |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(!!(after & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ))
	{
		if(m_depthStencil)
		{
			dstStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
				| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dstAccesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}
		else
		{
			dstStages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstAccesses |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
	}

	if(!!(after & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE))
	{
		if(m_depthStencil)
		{
			dstStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
				| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dstAccesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else
		{
			dstStages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstAccesses |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
	}

	if(!!(after & TextureUsageBit::GENERATE_MIPMAPS))
	{
		dstStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;

		if(level == 0)
		{
			dstAccesses |= VK_ACCESS_TRANSFER_READ_BIT;
		}
		else
		{
			ANKI_ASSERT(0 && "This will happen in generateMipmaps");
		}
	}

	if(!!(after & TextureUsageBit::UPLOAD))
	{
		dstStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstAccesses |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(!!(after & TextureUsageBit::CLEAR))
	{
		dstStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstAccesses |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	ANKI_ASSERT(dstStages);
}

//==============================================================================
VkImageLayout TextureImpl::computeLayout(TextureUsageBit usage, U level) const
{
	ANKI_ASSERT(level < m_mipCount);
	ANKI_ASSERT(usageValid(usage));

	VkImageLayout out = VK_IMAGE_LAYOUT_MAX_ENUM;
	Bool lastLevel = level == m_mipCount - 1u;

	if(usage == TextureUsageBit::NONE)
	{
		out = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	else if(!!(usage & TextureUsageBit::SAMPLED_ALL)
		&& !(usage & ~TextureUsageBit::SAMPLED_ALL))
	{
		out = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else if(!!(usage & TextureUsageBit::IMAGE_COMPUTE_READ_WRITE)
		&& !(usage & ~TextureUsageBit::IMAGE_COMPUTE_READ_WRITE))
	{
		out = VK_IMAGE_LAYOUT_GENERAL;
	}
	else if(usage == TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
		|| usage == TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE)
	{
		if(m_depthStencil)
		{
			out = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			out = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}
	else if(m_depthStencil
		&& !!(usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ)
		&& !!(usage & TextureUsageBit::SAMPLED_ALL_GRAPHICS)
		&& !(usage
				& ~(TextureUsageBit::SAMPLED_ALL_GRAPHICS
					  | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ)))
	{
		out = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	}
	else if(usage == TextureUsageBit::GENERATE_MIPMAPS)
	{
		if(!lastLevel)
		{
			out = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}
		else
		{
			out = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}
	}
	else if(usage == TextureUsageBit::CLEAR)
	{
		out = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	else if(!m_depthStencil && usage == TextureUsageBit::UPLOAD)
	{
		out = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}

	ANKI_ASSERT(out != VK_IMAGE_LAYOUT_MAX_ENUM);
	return out;
}

} // end namespace anki
