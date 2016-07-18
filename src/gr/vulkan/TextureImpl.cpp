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

	if(init.m_mipmapsCount > 1)
	{
		// May be used for mip gen.
		flags |=
			VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT;
	}

	if((init.m_usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE)
		!= TextureUsageBit::NONE)
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

	// This is quite common
	flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;

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
Error TextureImpl::init(const TextureInitInfo& init, Texture* tex)
{
	ANKI_ASSERT(init.isValid());
	m_sampler = getGrManager().newInstanceCached<Sampler>(init.m_sampling);
	m_width = init.m_width;
	m_height = init.m_height;
	m_format = init.m_format;

	CreateContext ctx;
	ctx.m_init = init;
	ANKI_CHECK(initImage(ctx));
	ANKI_CHECK(initView(ctx));

	// Transition the image layout from undefined to something relevant
	m_usage = init.m_usage;
	ANKI_ASSERT((init.m_initialUsage & TextureUsageBit::GENERATE_MIPMAPS)
			== TextureUsageBit::NONE
		&& "Can't handle that as initial");
	VkImageLayout initialLayout = computeLayout(init.m_initialUsage,
		formatIsDepthStencil(init.m_format),
		0,
		m_mipCount);

	if(initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
	{
		CommandBufferInitInfo cmdbinit;
		cmdbinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
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
			init.m_format.m_components = ComponentFormat::R8G8B8A8;
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

	VkImageCreateFlags flags = 0;
	if(init.m_type == TextureType::CUBE
		|| init.m_type == TextureType::CUBE_ARRAY)
	{
		flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VkImageCreateInfo& ci = ctx.m_imgCi;
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.flags = flags;
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
		ANKI_ASSERT(init.m_depth == 1);
		break;
	case TextureType::_2D_ARRAY:
		ci.extent.depth = 1;
		ci.arrayLayers = init.m_depth;
		break;
	case TextureType::CUBE:
		ci.extent.depth = 1;
		ci.arrayLayers = 6;
		ANKI_ASSERT(init.m_depth == 1);
		break;
	case TextureType::CUBE_ARRAY:
		ci.extent.depth = 1;
		ci.arrayLayers = 6 * init.m_depth;
		break;
	case TextureType::_3D:
		ci.extent.depth = init.m_depth;
		ci.arrayLayers = 1;
		break;
	default:
		ANKI_ASSERT(0);
	}

	m_layerCount = ci.arrayLayers;

	ci.mipLevels = init.m_mipmapsCount;
	ANKI_ASSERT(ci.mipLevels > 0);
	m_mipCount = init.m_mipmapsCount;

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

	if(formatIsDepthStencil(init.m_format))
	{
		m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		m_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	}

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

	return ErrorCode::NONE;
}

} // end namespace anki
