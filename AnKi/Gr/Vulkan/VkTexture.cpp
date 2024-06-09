// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkTexture.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Gr/Vulkan/VkDescriptor.h>

namespace anki {

Texture* Texture::newInstance(const TextureInitInfo& init)
{
	TextureImpl* impl = anki::newInstance<TextureImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

U32 Texture::getOrCreateBindlessTextureIndex(const TextureSubresourceDescriptor& subresource)
{
	ANKI_VK_SELF(TextureImpl);

	ANKI_ASSERT(TextureView(this, subresource).isGoodForSampling());

	const TextureImpl::TextureViewEntry& entry = self.getTextureViewEntry(subresource);

	LockGuard lock(entry.m_bindlessIndexLock);

	if(entry.m_bindlessIndex == kMaxU32) [[unlikely]]
	{
		entry.m_bindlessIndex = BindlessDescriptorSet::getSingleton().bindTexture(entry.m_handle, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	return entry.m_bindlessIndex;
}

static Bool isAstcLdrFormat(const VkFormat format)
{
	return format >= VK_FORMAT_ASTC_4x4_UNORM_BLOCK && format <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
}

static Bool isAstcSrgbFormat(const VkFormat format)
{
	switch(format)
	{
	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
		return true;
	default:
		return false;
	}
}

TextureImpl::~TextureImpl()
{
#if ANKI_ASSERTIONS_ENABLED
	if(m_usage != m_usedFor)
	{
		ANKI_VK_LOGW("Texture %s hasn't been used in all types of usages", getName().cstr());
	}
#endif

	TextureGarbage* garbage = anki::newInstance<TextureGarbage>(GrMemoryPool::getSingleton());

	auto destroyTextureView = [&](TextureViewEntry& entry) {
		if(entry.m_handle)
		{
			garbage->m_viewHandles.emplaceBack(entry.m_handle);
		}

		if(entry.m_bindlessIndex != kMaxU32)
		{
			garbage->m_bindlessIndices.emplaceBack(entry.m_bindlessIndex);
		}
	};

	for(ViewClass c : EnumIterable<ViewClass>())
	{
		for(TextureViewEntry& entry : m_textureViews[c])
		{
			destroyTextureView(entry);
		}

		destroyTextureView(m_wholeTextureViews[c]);
	}

	if(m_imageHandle && !(m_usage & TextureUsageBit::kPresent))
	{
		garbage->m_imageHandle = m_imageHandle;
	}

	garbage->m_memoryHandle = m_memHandle;

	getGrManagerImpl().getFrameGarbageCollector().newTextureGarbage(garbage);
}

Error TextureImpl::initInternal(VkImage externalImage, const TextureInitInfo& init_)
{
	TextureInitInfo init = init_;
	ANKI_ASSERT(init.isValid());
	if(externalImage)
	{
		ANKI_ASSERT(!!(init.m_usage & TextureUsageBit::kPresent));
	}

	ANKI_ASSERT(getGrManagerImpl().getDeviceCapabilities().m_vrs || !(init.m_usage & TextureUsageBit::kFramebufferShadingRate));

	// Set some stuff
	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_texType = init.m_type;

	if(m_texType == TextureType::k3D)
	{
		m_mipCount = min<U32>(init.m_mipmapCount, computeMaxMipmapCount3d(m_width, m_height, m_depth));
	}
	else
	{
		m_mipCount = min<U32>(init.m_mipmapCount, computeMaxMipmapCount2d(m_width, m_height));
	}
	init.m_mipmapCount = U8(m_mipCount);

	m_layerCount = init.m_layerCount;

	m_format = init.m_format;
	m_vkFormat = convertFormat(m_format);
	m_aspect = getImageAspectFromFormat(m_format);
	m_usage = init.m_usage;

	if(externalImage)
	{
		m_imageHandle = externalImage;
	}
	else
	{
		ANKI_CHECK(initImage(init));
	}

	ANKI_CHECK(initViews());

	return Error::kNone;
}

VkImageCreateFlags TextureImpl::calcCreateFlags(const TextureInitInfo& init)
{
	VkImageCreateFlags flags = 0;
	if(init.m_type == TextureType::kCube || init.m_type == TextureType::kCubeArray)
	{
		flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	return flags;
}

Bool TextureImpl::imageSupported(const TextureInitInfo& init)
{
	VkImageFormatProperties props = {};

	const VkResult res = vkGetPhysicalDeviceImageFormatProperties(getGrManagerImpl().getPhysicalDevice(), m_vkFormat, convertTextureType(init.m_type),
																  VK_IMAGE_TILING_OPTIMAL, convertTextureUsage(init.m_usage, init.m_format),
																  calcCreateFlags(init), &props);

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

Error TextureImpl::initImage(const TextureInitInfo& init)
{
	// Check if format is supported
	if(!imageSupported(init))
	{
		ANKI_VK_LOGE("TextureInitInfo contains a combination of parameters that it's not supported by the device. "
					 "Texture format is %s",
					 getFormatInfo(init.m_format).m_name);
		return Error::kFunctionFailed;
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
	case TextureType::k1D:
	case TextureType::k2D:
		ci.extent.depth = 1;
		ci.arrayLayers = 1;
		break;
	case TextureType::k2DArray:
		ci.extent.depth = 1;
		ci.arrayLayers = init.m_layerCount;
		break;
	case TextureType::kCube:
		ci.extent.depth = 1;
		ci.arrayLayers = 6;
		break;
	case TextureType::kCubeArray:
		ci.extent.depth = 1;
		ci.arrayLayers = 6 * init.m_layerCount;
		break;
	case TextureType::k3D:
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
	m_vkUsageFlags = ci.usage;
	ci.queueFamilyIndexCount = getGrManagerImpl().getQueueFamilies().getSize();
	ci.pQueueFamilyIndices = &getGrManagerImpl().getQueueFamilies()[0];
	ci.sharingMode = (ci.queueFamilyIndexCount > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	ANKI_VK_CHECK(vkCreateImage(getVkDevice(), &ci, nullptr, &m_imageHandle));
	getGrManagerImpl().trySetVulkanHandleName(init.getName(), VK_OBJECT_TYPE_IMAGE, m_imageHandle);
#if 0
	printf("Creating texture %p %s\n", static_cast<void*>(m_imageHandle),
		   init.getName() ? init.getName().cstr() : "Unnamed");
#endif

	// Allocate memory
	//
	VkMemoryDedicatedRequirementsKHR dedicatedRequirements = {};
	dedicatedRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR;

	VkMemoryRequirements2 requirements = {};
	requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
	requirements.pNext = &dedicatedRequirements;

	VkImageMemoryRequirementsInfo2 imageRequirementsInfo = {};
	imageRequirementsInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
	imageRequirementsInfo.image = m_imageHandle;

	vkGetImageMemoryRequirements2(getVkDevice(), &imageRequirementsInfo, &requirements);

	U32 memIdx = GpuMemoryManager::getSingleton().findMemoryType(requirements.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
																 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Fallback
	if(memIdx == kMaxU32)
	{
		memIdx =
			GpuMemoryManager::getSingleton().findMemoryType(requirements.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
	}

	ANKI_ASSERT(memIdx != kMaxU32);

	// Allocate
	if(!dedicatedRequirements.prefersDedicatedAllocation)
	{
		GpuMemoryManager::getSingleton().allocateMemory(memIdx, requirements.memoryRequirements.size, U32(requirements.memoryRequirements.alignment),
														m_memHandle);
	}
	else
	{
		GpuMemoryManager::getSingleton().allocateMemoryDedicated(memIdx, requirements.memoryRequirements.size, m_imageHandle, m_memHandle);
	}

	// Bind
	ANKI_VK_CHECK(vkBindImageMemory(getVkDevice(), m_imageHandle, m_memHandle.m_memory, m_memHandle.m_offset));

	return Error::kNone;
}

void TextureImpl::computeBarrierInfo(TextureUsageBit usage, Bool src, U32 level, VkPipelineStageFlags& stages, VkAccessFlags& accesses) const
{
	ANKI_ASSERT(level < m_mipCount);
	ANKI_ASSERT(usageValid(usage));
	stages = 0;
	accesses = 0;
	const Bool depthStencil = !!m_aspect;
	const Bool rt = getGrManagerImpl().getDeviceCapabilities().m_rayTracingEnabled;

	if(!!(usage & (TextureUsageBit::kSampledGeometry | TextureUsageBit::kStorageGeometryRead)))
	{
		stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
				  | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		accesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(usage & TextureUsageBit::kStorageGeometryWrite))
	{
		stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
				  | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		accesses |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(!!(usage & (TextureUsageBit::kSampledFragment | TextureUsageBit::kStorageFragmentRead)))
	{
		stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		accesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(usage & TextureUsageBit::kStorageFragmentWrite))
	{
		stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		accesses |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(!!(usage & (TextureUsageBit::kSampledCompute | TextureUsageBit::kStorageComputeRead)))
	{
		stages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		accesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(usage & TextureUsageBit::kStorageComputeWrite))
	{
		stages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		accesses |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(!!(usage & (TextureUsageBit::kSampledTraceRays | TextureUsageBit::kStorageTraceRaysRead)) && rt)
	{
		stages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		accesses |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(!!(usage & TextureUsageBit::kStorageTraceRaysWrite) && rt)
	{
		stages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		accesses |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(!!(usage & TextureUsageBit::kFramebufferRead))
	{
		if(depthStencil)
		{
			stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			accesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}
		else
		{
			stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			accesses |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
	}

	if(!!(usage & TextureUsageBit::kFramebufferWrite))
	{
		if(depthStencil)
		{
			stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			accesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else
		{
			stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			accesses |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
	}

	if(!!(usage & TextureUsageBit::kFramebufferShadingRate))
	{
		stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
		accesses |= VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
	}

	if(!!(usage & TextureUsageBit::kGenerateMipmaps))
	{
		stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		if(src)
		{
			const Bool lastLevel = level == m_mipCount - 1;
			if(lastLevel)
			{
				accesses |= VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			else
			{
				accesses |= VK_ACCESS_TRANSFER_READ_BIT;
			}
		}
		else
		{
			ANKI_ASSERT(level == 0
						&& "The upper layers should not allow others levels to transition to gen mips state. This "
						   "happens elsewhere");
			accesses |= VK_ACCESS_TRANSFER_READ_BIT;
		}
	}

	if(!!(usage & TextureUsageBit::kTransferDestination))
	{
		stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		accesses |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(!!(usage & TextureUsageBit::kPresent))
	{
		stages |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		accesses |= VK_ACCESS_MEMORY_READ_BIT;
	}
}

void TextureImpl::computeBarrierInfo(TextureUsageBit before, TextureUsageBit after, U32 level, VkPipelineStageFlags& srcStages,
									 VkAccessFlags& srcAccesses, VkPipelineStageFlags& dstStages, VkAccessFlags& dstAccesses) const
{
	computeBarrierInfo(before, true, level, srcStages, srcAccesses);
	computeBarrierInfo(after, false, level, dstStages, dstAccesses);

	if(srcStages == 0)
	{
		srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
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

	if(usage == TextureUsageBit::kNone)
	{
		out = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	else if(depthStencil)
	{
		if(!(usage & ~(TextureUsageBit::kAllSampled | TextureUsageBit::kFramebufferRead)))
		{
			// Only depth tests and sampled
			out = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
		else
		{
			// Only attachment write, the rest (eg transfer) are not supported for now
			ANKI_ASSERT(usage == TextureUsageBit::kFramebufferWrite || usage == TextureUsageBit::kAllFramebuffer);
			out = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}
	else if(!(usage & ~TextureUsageBit::kAllFramebuffer))
	{
		// Color attachment
		out = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	else if(!(usage & ~TextureUsageBit::kFramebufferShadingRate))
	{
		// SRI
		out = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
	}
	else if(!(usage & ~TextureUsageBit::kAllStorage))
	{
		// Only image load/store
		out = VK_IMAGE_LAYOUT_GENERAL;
	}
	else if(!(usage & ~TextureUsageBit::kAllSampled))
	{
		// Only sampled
		out = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else if(usage == TextureUsageBit::kGenerateMipmaps)
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
	else if(usage == TextureUsageBit::kTransferDestination)
	{
		out = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	else if(usage == TextureUsageBit::kPresent)
	{
		out = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	else
	{
		// No idea so play it safe
		out = VK_IMAGE_LAYOUT_GENERAL;
	}

	return out;
}

Error TextureImpl::initViews()
{
	const VkImageAspectFlags vkaspect = convertImageAspect(m_aspect);

	VkImageViewCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ci.image = m_imageHandle;
	ci.viewType = convertTextureViewType(m_texType);
	ci.format = m_vkFormat;
	ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.subresourceRange.aspectMask = vkaspect;
	ci.subresourceRange.baseArrayLayer = 0;
	ci.subresourceRange.baseMipLevel = 0;
	ci.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	ci.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

	VkImageViewASTCDecodeModeEXT astcDecodeMode;
	if(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_astc_decode_mode) && isAstcLdrFormat(m_vkFormat)
	   && !isAstcSrgbFormat(m_vkFormat))
	{
		astcDecodeMode = {};
		astcDecodeMode.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT;
		astcDecodeMode.decodeMode = VK_FORMAT_R8G8B8A8_UNORM;

		appendPNextList(ci, &astcDecodeMode);
	}

	auto createImageView = [&](VkImageView& view) -> Error {
		ANKI_VK_CHECK(vkCreateImageView(getVkDevice(), &ci, nullptr, &view));
		getGrManagerImpl().trySetVulkanHandleName(getName(), VK_OBJECT_TYPE_IMAGE_VIEW, view);
		return Error::kNone;
	};

	ANKI_CHECK(createImageView(m_wholeTextureViews[ViewClass::kDefault].m_handle));
	if(m_aspect == DepthStencilAspectBit::kDepthStencil)
	{
		ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		ANKI_CHECK(createImageView(m_wholeTextureViews[ViewClass::kDepth].m_handle));

		ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		ANKI_CHECK(createImageView(m_wholeTextureViews[ViewClass::kStencil].m_handle));
	}

	// Create the rest of the views
	const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;
	const Bool needsMoreViews = m_layerCount > 1 || m_mipCount > 1 || faceCount > 1;

	if(needsMoreViews)
	{
		m_textureViews[ViewClass::kDefault].resize(m_layerCount * faceCount * m_mipCount);

		if(m_aspect == DepthStencilAspectBit::kDepthStencil)
		{
			m_textureViews[ViewClass::kDefault].resize(m_layerCount * faceCount * m_mipCount);
			m_textureViews[ViewClass::kStencil].resize(m_layerCount * faceCount * m_mipCount);
		}

		const TextureType surfaceOrVolumeTexType = (m_texType == TextureType::k3D) ? TextureType::k3D : TextureType::k2D;
		ci.viewType = convertTextureViewType(surfaceOrVolumeTexType);
		ci.subresourceRange.layerCount = 1;
		ci.subresourceRange.levelCount = 1;

		for(U32 layer = 0; layer < m_layerCount; ++layer)
		{
			for(U32 face = 0; face < faceCount; ++face)
			{
				for(U32 mip = 0; mip < m_mipCount; ++mip)
				{
					const U32 idx = translateSurfaceOrVolume(layer, face, mip);
					TextureViewEntry& entry = m_textureViews[ViewClass::kDefault][idx];

					ci.subresourceRange.baseArrayLayer = layer * faceCount + face;
					ci.subresourceRange.baseMipLevel = mip;
					ci.subresourceRange.aspectMask = vkaspect;

					ANKI_CHECK(createImageView(entry.m_handle));

					if(m_textureViews[ViewClass::kDepth].getSize())
					{
						ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
						ANKI_CHECK(createImageView(m_textureViews[ViewClass::kDepth][idx].m_handle));
					}

					if(m_textureViews[ViewClass::kStencil].getSize())
					{
						ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
						ANKI_CHECK(createImageView(m_textureViews[ViewClass::kStencil][idx].m_handle));
					}
				}
			}
		}
	}

	return Error::kNone;
}

const TextureImpl::TextureViewEntry& TextureImpl::getTextureViewEntry(const TextureSubresourceDescriptor& subresource) const
{
	const TextureView view(this, subresource);

	// Find class
	ViewClass c;
	if(view.getDepthStencilAspect() == m_aspect)
	{
		c = ViewClass::kDefault;
	}
	else if(view.getDepthStencilAspect() == DepthStencilAspectBit::kDepth)
	{
		c = ViewClass::kDepth;
	}
	else
	{
		ANKI_ASSERT(view.getDepthStencilAspect() == DepthStencilAspectBit::kStencil);
		c = ViewClass::kStencil;
	}

	// Get
	if(view.isAllSurfacesOrVolumes())
	{
		return m_wholeTextureViews[c];
	}
	else
	{
		const U32 idx = translateSurfaceOrVolume(view.getFirstLayer(), view.getFirstFace(), view.getFirstMipmap());
		return m_textureViews[c][idx];
	}
}

} // end namespace anki
