// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/gr/GrObjectCache.h>

namespace anki
{

TextureImpl::TextureImpl(GrManager* manager, U64 uuid)
	: VulkanObject(manager)
	, m_uuid(uuid)
{
}

TextureImpl::~TextureImpl()
{
	for(auto it : m_viewsMap)
	{
		if(it != VK_NULL_HANDLE)
		{
			vkDestroyImageView(getDevice(), it, nullptr);
		}
	}

	m_viewsMap.destroy(getAllocator());

	if(m_imageHandle)
	{
		vkDestroyImage(getDevice(), m_imageHandle, nullptr);
	}

	if(m_memHandle)
	{
		getGrManagerImpl().getGpuMemoryManager().freeMemory(m_memHandle);
	}
}

Error TextureImpl::init(const TextureInitInfo& init_, Texture* tex)
{
	TextureInitInfo init = init_;
	ANKI_ASSERT(textureInitInfoValid(init));
	m_sampler = getGrManagerImpl().getSamplerCache().newInstance<Sampler>(init.m_sampling);

	// Set some stuff
	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_type = init.m_type;

	if(m_type == TextureType::_3D)
	{
		m_mipCount = min<U>(init.m_mipmapsCount, computeMaxMipmapCount3d(m_width, m_height, m_depth));
	}
	else
	{
		m_mipCount = min<U>(init.m_mipmapsCount, computeMaxMipmapCount2d(m_width, m_height));
	}
	init.m_mipmapsCount = m_mipCount;

	m_layerCount = init.m_layerCount;

	m_format = init.m_format;
	m_vkFormat = convertFormat(m_format);
	m_depthStencil = formatIsDepthStencil(m_format);
	m_aspect = convertImageAspect(m_format);
	m_usage = init.m_usage;
	m_usageWhenEncountered = init.m_usageWhenEncountered;
	ANKI_ASSERT((m_usageWhenEncountered | m_usage) == m_usage);

	if(m_aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
	{
		m_akAspect |= DepthStencilAspectBit::DEPTH;
	}
	if(m_aspect & VK_IMAGE_ASPECT_STENCIL_BIT)
	{
		m_akAspect |= DepthStencilAspectBit::STENCIL;
	}

	ANKI_CHECK(initImage(init));

	// Init the template
	memset(&m_viewCreateInfoTemplate, 0, sizeof(m_viewCreateInfoTemplate)); // memset, it will be used for hashing
	m_viewCreateInfoTemplate.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	m_viewCreateInfoTemplate.image = m_imageHandle;
	m_viewCreateInfoTemplate.viewType = convertTextureViewType(init.m_type);
	m_viewCreateInfoTemplate.format = m_vkFormat;
	m_viewCreateInfoTemplate.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	m_viewCreateInfoTemplate.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	m_viewCreateInfoTemplate.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	m_viewCreateInfoTemplate.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	m_viewCreateInfoTemplate.subresourceRange.aspectMask = m_aspect;
	m_viewCreateInfoTemplate.subresourceRange.baseArrayLayer = 0;
	m_viewCreateInfoTemplate.subresourceRange.baseMipLevel = 0;
	m_viewCreateInfoTemplate.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	m_viewCreateInfoTemplate.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

	// Transition the image layout from undefined to something relevant
	if(!!init.m_initialUsage)
	{
		ANKI_ASSERT(usageValid(init.m_initialUsage));
		ANKI_ASSERT(!(init.m_initialUsage & TextureUsageBit::GENERATE_MIPMAPS) && "That doesn't make any sense");

		CommandBufferInitInfo cmdbinit;
		cmdbinit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SMALL_BATCH;
		CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(cmdbinit);

		VkImageSubresourceRange range;
		range.aspectMask = m_aspect;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = m_layerCount;
		range.levelCount = m_mipCount;

		cmdb->m_impl->setTextureBarrierRange(TexturePtr(tex), TextureUsageBit::NONE, init.m_initialUsage, range);

		cmdb->m_impl->endRecording();
		getGrManagerImpl().flushCommandBuffer(cmdb);
	}

	return ErrorCode::NONE;
}

VkFormatFeatureFlags TextureImpl::calcFeatures(const TextureInitInfo& init)
{
	VkFormatFeatureFlags flags = 0;

	if(init.m_mipmapsCount > 1 && !!(init.m_usage & TextureUsageBit::GENERATE_MIPMAPS))
	{
		// May be used for mip gen.
		flags |= VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT;
	}

	if(!!(init.m_usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE))
	{
		if(formatIsDepthStencil(init.m_format))
		{
			flags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else
		{
			flags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
		}
	}

	if(!!(init.m_usage & TextureUsageBit::SAMPLED_ALL))
	{
		flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	}

	ANKI_ASSERT(flags);
	return flags;
}

VkImageCreateFlags TextureImpl::calcCreateFlags(const TextureInitInfo& init)
{
	VkImageCreateFlags flags = 0;
	if(init.m_type == TextureType::CUBE || init.m_type == TextureType::CUBE_ARRAY)
	{
		flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	return flags;
}

Bool TextureImpl::imageSupported(const TextureInitInfo& init)
{
	VkImageFormatProperties props = {};

	VkResult res = vkGetPhysicalDeviceImageFormatProperties(getGrManagerImpl().getPhysicalDevice(),
		m_vkFormat,
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

Error TextureImpl::initImage(const TextureInitInfo& init_)
{
	TextureInitInfo init = init_;

	// Check if format is supported
	Bool supported;
	while(!(supported = imageSupported(init)))
	{
		// Try to find a fallback
		if(init.m_format.m_components == ComponentFormat::R8G8B8)
		{
			ANKI_ASSERT(!(init.m_usage & TextureUsageBit::IMAGE_ALL) && "Can't do that ATM");
			init.m_format.m_components = ComponentFormat::R8G8B8A8;
			m_format = init.m_format;
			m_vkFormat = convertFormat(m_format);
			m_workarounds = TextureImplWorkaround::R8G8B8_TO_R8G8B8A8;
		}
		else if(init.m_format.m_components == ComponentFormat::S8)
		{
			ANKI_ASSERT(
				!(init.m_usage & (TextureUsageBit::IMAGE_ALL | TextureUsageBit::TRANSFER_ANY)) && "Can't do that ATM");
			init.m_format = PixelFormat(ComponentFormat::D24S8, TransformFormat::UNORM);
			m_format = init.m_format;
			m_vkFormat = convertFormat(m_format);
			m_workarounds = TextureImplWorkaround::S8_TO_D24S8;
			m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			m_akAspect = DepthStencilAspectBit::DEPTH | DepthStencilAspectBit::STENCIL;
		}
		else if(init.m_format.m_components == ComponentFormat::D24S8)
		{
			ANKI_ASSERT(
				!(init.m_usage & (TextureUsageBit::IMAGE_ALL | TextureUsageBit::TRANSFER_ANY)) && "Can't do that ATM");
			init.m_format = PixelFormat(ComponentFormat::D32S8, TransformFormat::UNORM);
			m_format = init.m_format;
			m_vkFormat = convertFormat(m_format);
			m_workarounds = TextureImplWorkaround::D24S8_TO_D32S8;
			m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			m_akAspect = DepthStencilAspectBit::DEPTH | DepthStencilAspectBit::STENCIL;
		}
		else
		{
			break;
		}
	}

	if(!supported)
	{
		ANKI_VK_LOGE("Unsupported texture format: %u %u", U(init.m_format.m_components), U(init.m_format.m_transform));
		return ErrorCode::FUNCTION_FAILED;
	}

	// Contunue with the creation
	VkImageCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.flags = calcCreateFlags(init);
	ci.imageType = convertTextureType(m_type);
	ci.format = m_vkFormat;
	ci.extent.width = init.m_width;
	ci.extent.height = init.m_height;

	switch(m_type)
	{
	case TextureType::_1D:
	case TextureType::_2D:
		ci.extent.depth = 1;
		ci.arrayLayers = 1;

		m_surfaceOrVolumeCount = m_mipCount;
		break;
	case TextureType::_2D_ARRAY:
		ci.extent.depth = 1;
		ci.arrayLayers = init.m_layerCount;

		m_surfaceOrVolumeCount = m_mipCount * m_layerCount;
		break;
	case TextureType::CUBE:
		ci.extent.depth = 1;
		ci.arrayLayers = 6;

		m_surfaceOrVolumeCount = m_mipCount * 6;
		break;
	case TextureType::CUBE_ARRAY:
		ci.extent.depth = 1;
		ci.arrayLayers = 6 * init.m_layerCount;

		m_surfaceOrVolumeCount = m_mipCount * 6 * m_layerCount;
		break;
	case TextureType::_3D:
		ci.extent.depth = init.m_depth;
		ci.arrayLayers = 1;

		m_surfaceOrVolumeCount = m_mipCount;
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
	getGrManagerImpl().trySetVulkanHandleName(
		init.m_name, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, ptrToNumber(m_imageHandle));

	// Allocate memory
	//
	VkMemoryRequirements req = {};
	vkGetImageMemoryRequirements(getDevice(), m_imageHandle, &req);

	U memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
		req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Fallback
	if(memIdx == MAX_U32)
	{
		memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
			req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
	}

	ANKI_ASSERT(memIdx != MAX_U32);

	// Allocate
	getGrManagerImpl().getGpuMemoryManager().allocateMemory(memIdx, req.size, req.alignment, false, m_memHandle);

	// Bind mem to image
	ANKI_VK_CHECK(vkBindImageMemory(getDevice(), m_imageHandle, m_memHandle.m_memory, m_memHandle.m_offset));

	return ErrorCode::NONE;
}

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

	if(!!(before & TextureUsageBit::SAMPLED_COMPUTE) || !!(before & TextureUsageBit::IMAGE_COMPUTE_READ))
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
			srcStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
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

	if(!!(before & TextureUsageBit::TRANSFER_DESTINATION))
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

	if(!!(after & TextureUsageBit::SAMPLED_COMPUTE) || !!(after & TextureUsageBit::IMAGE_COMPUTE_READ))
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
			dstStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
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
			dstStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
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

	if(!!(after & TextureUsageBit::TRANSFER_DESTINATION))
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
	else if(!(usage & ~TextureUsageBit::SAMPLED_ALL))
	{
		// Only sampling
		out = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else if(!(usage & ~TextureUsageBit::IMAGE_COMPUTE_READ_WRITE))
	{
		// Only image load/store
		out = VK_IMAGE_LAYOUT_GENERAL;
	}
	else if(!(usage & ~TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE))
	{
		// Only FB access
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
		&& !(usage & ~(TextureUsageBit::SAMPLED_ALL_GRAPHICS | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ)))
	{
		// FB read & shader read
		out = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	}
	else if(m_depthStencil
		&& !(usage & ~(TextureUsageBit::SAMPLED_ALL_GRAPHICS | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE)))
	{
		// Wild guess: One aspect is shader read and the other is read write
		out = VK_IMAGE_LAYOUT_GENERAL;
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
	else if(!m_depthStencil && usage == TextureUsageBit::TRANSFER_DESTINATION)
	{
		out = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}

	ANKI_ASSERT(out != VK_IMAGE_LAYOUT_MAX_ENUM);
	return out;
}

VkImageView TextureImpl::getOrCreateSingleSurfaceView(const TextureSurfaceInfo& surf, DepthStencilAspectBit aspect)
{
	checkSurfaceOrVolume(surf);

	VkImageViewCreateInfo ci = m_viewCreateInfoTemplate;
	ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	computeSubResourceRange(surf, aspect, ci.subresourceRange);

	return getOrCreateView(ci);
}

VkImageView TextureImpl::getOrCreateResourceGroupView(DepthStencilAspectBit aspect)
{
	VkImageViewCreateInfo ci = m_viewCreateInfoTemplate;
	ci.subresourceRange.aspectMask = convertAspect(aspect);

	return getOrCreateView(ci);
}

VkImageView TextureImpl::getOrCreateView(const VkImageViewCreateInfo& ci)
{
	LockGuard<Mutex> lock(m_viewsMapMtx);
	auto it = m_viewsMap.find(ci);

	if(it != m_viewsMap.getEnd())
	{
		return *it;
	}
	else
	{
		VkImageView view = VK_NULL_HANDLE;
		ANKI_VK_CHECKF(vkCreateImageView(getDevice(), &ci, nullptr, &view));
		m_viewsMap.pushBack(getAllocator(), ci, view);

		return view;
	}
}

} // end namespace anki
