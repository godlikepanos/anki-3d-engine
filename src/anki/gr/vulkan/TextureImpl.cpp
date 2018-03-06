// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

	if(m_dedicatedMem)
	{
		vkFreeMemory(getDevice(), m_dedicatedMem, nullptr);
	}
}

Error TextureImpl::init(const TextureInitInfo& init_)
{
	TextureInitInfo init = init_;
	ANKI_ASSERT(init.isValid());

	// Set some stuff
	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_texType = init.m_type;

	if(m_texType == TextureType::_3D)
	{
		m_mipCount = min<U>(init.m_mipmapCount, computeMaxMipmapCount3d(m_width, m_height, m_depth));
	}
	else
	{
		m_mipCount = min<U>(init.m_mipmapCount, computeMaxMipmapCount2d(m_width, m_height));
	}
	init.m_mipmapCount = m_mipCount;

	m_layerCount = init.m_layerCount;

	m_format = init.m_format;
	m_vkFormat = convertFormat(m_format);
	m_aspect = getImageAspectFromFormat(m_format);
	m_usage = init.m_usage;

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
	m_viewCreateInfoTemplate.subresourceRange.aspectMask = convertImageAspect(m_aspect);
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
		CommandBufferPtr cmdb = getManager().newCommandBuffer(cmdbinit);

		VkImageSubresourceRange range;
		range.aspectMask = convertImageAspect(m_aspect);
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = m_layerCount;
		range.levelCount = m_mipCount;

		static_cast<CommandBufferImpl&>(*cmdb).setTextureBarrierRange(
			TexturePtr(this), TextureUsageBit::NONE, init.m_initialUsage, range);

		static_cast<CommandBufferImpl&>(*cmdb).endRecording();
		getGrManagerImpl().flushCommandBuffer(cmdb, nullptr);
	}

	return Error::NONE;
}

VkFormatFeatureFlags TextureImpl::calcFeatures(const TextureInitInfo& init)
{
	VkFormatFeatureFlags flags = 0;

	if(init.m_mipmapCount > 1 && !!(init.m_usage & TextureUsageBit::GENERATE_MIPMAPS))
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
	Bool useDedicatedMemory = !!(init.m_usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE)
							  && !!(getGrManagerImpl().getExtensions() & VulkanExtensions::NV_DEDICATED_ALLOCATION);

	// Check if format is supported
	Bool supported;
	while(!(supported = imageSupported(init)))
	{
		// Try to find a fallback
		if(init.m_format >= Format::R8G8B8_UNORM && init.m_format <= Format::R8G8B8_SRGB)
		{
			ANKI_ASSERT(!(init.m_usage & TextureUsageBit::IMAGE_ALL) && "Can't do that ATM");
			const U idx = U(init.m_format) - U(Format::R8G8B8_UNORM);
			init.m_format = Format(U(Format::R8G8B8A8_UNORM) + idx);
			ANKI_ASSERT(init.m_format >= Format::R8G8B8A8_UNORM && init.m_format <= Format::R8G8B8A8_SRGB);
			m_format = init.m_format;
			m_vkFormat = convertFormat(m_format);
			m_workarounds = TextureImplWorkaround::R8G8B8_TO_R8G8B8A8;
		}
		else if(init.m_format == Format::S8_UINT)
		{
			ANKI_ASSERT(
				!(init.m_usage & (TextureUsageBit::IMAGE_ALL | TextureUsageBit::TRANSFER_ANY)) && "Can't do that ATM");
			init.m_format = Format::D24_UNORM_S8_UINT;
			m_format = init.m_format;
			m_vkFormat = convertFormat(m_format);
			m_workarounds = TextureImplWorkaround::S8_TO_D24S8;
		}
		else if(init.m_format == Format::D24_UNORM_S8_UINT)
		{
			ANKI_ASSERT(
				!(init.m_usage & (TextureUsageBit::IMAGE_ALL | TextureUsageBit::TRANSFER_ANY)) && "Can't do that ATM");
			init.m_format = Format::D32_SFLOAT_S8_UINT;
			m_format = init.m_format;
			m_vkFormat = convertFormat(m_format);
			m_workarounds = TextureImplWorkaround::D24S8_TO_D32S8;
		}
		else
		{
			break;
		}
	}

	if(!supported)
	{
		ANKI_VK_LOGE("Unsupported texture format: %u", U(init.m_format));
		return Error::FUNCTION_FAILED;
	}

	// Contunue with the creation
	VkImageCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.flags = calcCreateFlags(init);
	ci.imageType = convertTextureType(m_texType);
	ci.format = m_vkFormat;
	ci.extent.width = init.m_width;
	ci.extent.height = init.m_height;

	switch(m_texType)
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

	VkDedicatedAllocationImageCreateInfoNV dedicatedMemCi = {};
	if(useDedicatedMemory)
	{
		dedicatedMemCi.sType = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV;
		dedicatedMemCi.dedicatedAllocation = true;
		ci.pNext = &dedicatedMemCi;
	}

	ANKI_VK_CHECK(vkCreateImage(getDevice(), &ci, nullptr, &m_imageHandle));
	getGrManagerImpl().trySetVulkanHandleName(init.getName(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_imageHandle);
#if 0
	printf("Creating texture %p %s\n",
		static_cast<void*>(m_imageHandle),
		init.getName() ? init.getName().cstr() : "Unnamed");
#endif

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

	if(!useDedicatedMemory)
	{
		// Allocate
		getGrManagerImpl().getGpuMemoryManager().allocateMemory(memIdx, req.size, req.alignment, false, m_memHandle);

		// Bind mem to image
		ANKI_TRACE_START_EVENT(VK_BIND_OBJECT);
		ANKI_VK_CHECK(vkBindImageMemory(getDevice(), m_imageHandle, m_memHandle.m_memory, m_memHandle.m_offset));
		ANKI_TRACE_STOP_EVENT(VK_BIND_OBJECT);
	}
	else
	{
		VkDedicatedAllocationMemoryAllocateInfoNV dedicatedInfo = {};
		dedicatedInfo.sType = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV;
		dedicatedInfo.image = m_imageHandle;

		VkMemoryAllocateInfo memAllocCi = {};
		memAllocCi.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocCi.pNext = &dedicatedInfo;
		memAllocCi.allocationSize = req.size;
		memAllocCi.memoryTypeIndex = memIdx;

		ANKI_VK_CHECK(vkAllocateMemory(getDevice(), &memAllocCi, nullptr, &m_dedicatedMem));
		getGrManagerImpl().trySetVulkanHandleName(
			init.getName(), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, ptrToNumber(m_dedicatedMem));

		ANKI_TRACE_START_EVENT(VK_BIND_OBJECT);
		ANKI_VK_CHECK(vkBindImageMemory(getDevice(), m_imageHandle, m_dedicatedMem, 0));
		ANKI_TRACE_STOP_EVENT(VK_BIND_OBJECT);
	}

	return Error::NONE;
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
	const Bool lastLevel = level == m_mipCount - 1u;
	const Bool depthStencil = !!m_aspect;

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
		if(depthStencil)
		{
			srcStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			srcAccesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}
		else
		{
			srcStages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // See Table 4 in the spec
			srcAccesses |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
	}

	if(!!(before & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE))
	{
		if(depthStencil)
		{
			srcStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			srcAccesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else
		{
			srcStages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
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
		if(depthStencil)
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
		if(depthStencil)
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
	const Bool lastLevel = level == m_mipCount - 1u;
	const Bool depthStencil = !!m_aspect;

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
		if(depthStencil)
		{
			out = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			out = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}
	else if(depthStencil
			&& !(usage & ~(TextureUsageBit::SAMPLED_ALL_GRAPHICS | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ)))
	{
		// FB read & shader read
		out = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	}
	else if(depthStencil
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
	else if(!depthStencil && usage == TextureUsageBit::TRANSFER_DESTINATION)
	{
		out = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}

	ANKI_ASSERT(out != VK_IMAGE_LAYOUT_MAX_ENUM);
	return out;
}

VkImageView TextureImpl::getOrCreateView(const TextureSubresourceInfo& subresource, TextureType& newTexType) const
{
	LockGuard<Mutex> lock(m_viewsMapMtx);
	auto it = m_viewsMap.find(subresource);

	if(it != m_viewsMap.getEnd())
	{
		// Fixup the image view type
		newTexType = computeNewTexTypeOfSubresource(subresource);

		return *it;
	}
	else
	{
		// Compute the VkImageViewCreateInfo
		VkImageViewCreateInfo viewCi;
		computeVkImageViewCreateInfo(subresource, viewCi, newTexType);

		VkImageView view = VK_NULL_HANDLE;
		ANKI_VK_CHECKF(vkCreateImageView(getDevice(), &viewCi, nullptr, &view));
		getGrManagerImpl().trySetVulkanHandleName(
			getName(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, ptrToNumber(view));

		m_viewsMap.emplace(getAllocator(), subresource, view);

		return view;
	}
}

} // end namespace anki
