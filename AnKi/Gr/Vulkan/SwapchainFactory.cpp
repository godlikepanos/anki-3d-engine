// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/SwapchainFactory.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

namespace anki {

MicroSwapchain::MicroSwapchain(SwapchainFactory* factory)
	: m_factory(factory)
{
	ANKI_ASSERT(factory);

	if(initInternal())
	{
		ANKI_VK_LOGF("Error creating the swapchain. Will not try to recover");
	}
}

MicroSwapchain::~MicroSwapchain()
{
	const VkDevice dev = m_factory->m_gr->getDevice();

	m_textures.destroy(getMemoryPool());

	if(m_swapchain)
	{
		vkDestroySwapchainKHR(dev, m_swapchain, nullptr);
		m_swapchain = {};
	}
}

Error MicroSwapchain::initInternal()
{
	const VkDevice dev = m_factory->m_gr->getDevice();

	// Get the surface size
	VkSurfaceCapabilitiesKHR surfaceProperties;
	U32 surfaceWidth = 0, surfaceHeight = 0;
	{
		ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_factory->m_gr->getPhysicalDevice(),
																m_factory->m_gr->getSurface(), &surfaceProperties));

#if ANKI_WINDOWING_SYSTEM_HEADLESS
		if(surfaceProperties.currentExtent.width != kMaxU32 || surfaceProperties.currentExtent.height != kMaxU32)
		{
			ANKI_VK_LOGE("Was expecting an indication that the surface size will be determined by the extent of a "
						 "swapchain targeting the surface");
			return Error::kFunctionFailed;
		}
		m_factory->m_gr->getNativeWindowSize(surfaceWidth, surfaceHeight);
#else
		if(surfaceProperties.currentExtent.width == kMaxU32 || surfaceProperties.currentExtent.height == kMaxU32)
		{
			ANKI_VK_LOGE("Wrong surface size");
			return Error::kFunctionFailed;
		}
		surfaceWidth = surfaceProperties.currentExtent.width;
		surfaceHeight = surfaceProperties.currentExtent.height;
#endif
	}

	// Get the surface format
	VkFormat surfaceFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR colorspace = VK_COLOR_SPACE_MAX_ENUM_KHR;
	{
		uint32_t formatCount;
		ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_factory->m_gr->getPhysicalDevice(),
														   m_factory->m_gr->getSurface(), &formatCount, nullptr));

		DynamicArrayRaii<VkSurfaceFormatKHR> formats(&getMemoryPool());
		formats.create(formatCount);
		ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_factory->m_gr->getPhysicalDevice(),
														   m_factory->m_gr->getSurface(), &formatCount, &formats[0]));

		ANKI_VK_LOGV("Supported surface formats:");
		Format akSurfaceFormat = Format::kNone;
		for(U32 i = 0; i < formatCount; ++i)
		{
			const VkFormat vkFormat = formats[i].format;
			Format akFormat;
			switch(formats[i].format)
			{
#define ANKI_FORMAT_DEF(type, id, componentCount, texelSize, blockWidth, blockHeight, blockSize, shaderType, \
						depthStencil) \
	case id: \
		akFormat = Format::k##type; \
		break;
#include <AnKi/Gr/Format.defs.h>
#undef ANKI_FORMAT_DEF
			default:
				akFormat = Format::kNone;
			}

			ANKI_VK_LOGV("\t%s", (akFormat != Format::kNone) ? getFormatInfo(akFormat).m_name : "Unknown format");

			if(surfaceFormat == VK_FORMAT_UNDEFINED
			   && (vkFormat == VK_FORMAT_R8G8B8A8_UNORM || vkFormat == VK_FORMAT_B8G8R8A8_UNORM
				   || vkFormat == VK_FORMAT_A8B8G8R8_UNORM_PACK32))
			{
				surfaceFormat = vkFormat;
				colorspace = formats[i].colorSpace;
				akSurfaceFormat = akFormat;
			}
		}

		if(surfaceFormat == VK_FORMAT_UNDEFINED)
		{
			ANKI_VK_LOGE("Suitable surface format not found");
			return Error::kFunctionFailed;
		}
		else
		{
			ANKI_VK_LOGV("Selecting surface format %s", getFormatInfo(akSurfaceFormat).m_name);
		}
	}

	// Chose present mode
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
	VkPresentModeKHR presentModeSecondChoice = VK_PRESENT_MODE_MAX_ENUM_KHR;
	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_factory->m_gr->getPhysicalDevice(), m_factory->m_gr->getSurface(),
												  &presentModeCount, nullptr);
		presentModeCount = min(presentModeCount, 4u);
		Array<VkPresentModeKHR, 4> presentModes;
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_factory->m_gr->getPhysicalDevice(), m_factory->m_gr->getSurface(),
												  &presentModeCount, &presentModes[0]);

		if(m_factory->m_vsync)
		{
			for(U i = 0; i < presentModeCount; ++i)
			{
				if(presentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
				{
					presentMode = presentModes[i];
				}
				else if(presentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
				{
					presentModeSecondChoice = presentModes[i];
				}
			}
		}
		else
		{
			for(U i = 0; i < presentModeCount; ++i)
			{
				if(presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					presentMode = presentModes[i];
				}
				else if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					presentModeSecondChoice = presentModes[i];
				}
			}
		}

		presentMode = (presentMode != VK_PRESENT_MODE_MAX_ENUM_KHR) ? presentMode : presentModeSecondChoice;
		if(presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
		{
			ANKI_VK_LOGE("Couldn't find a present mode");
			return Error::kFunctionFailed;
		}
	}

	// Create swapchain
	{
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		if(surfaceProperties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		{
			compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		}
		else if(surfaceProperties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
		{
			compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
		}
		else if(surfaceProperties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
		{
			compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
		}
		else if(surfaceProperties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
		{
			compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
		}
		else
		{
			ANKI_VK_LOGE("Failed to set compositeAlpha");
			return Error::kFunctionFailed;
		}

		VkSwapchainCreateInfoKHR ci = {};
		ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		ci.surface = m_factory->m_gr->getSurface();
		ci.minImageCount = kMaxFramesInFlight;
		ci.imageFormat = surfaceFormat;
		ci.imageColorSpace = colorspace;
		ci.imageExtent.width = surfaceWidth;
		ci.imageExtent.height = surfaceHeight;
		ci.imageArrayLayers = 1;
		ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		ci.queueFamilyIndexCount = m_factory->m_gr->getQueueFamilies().getSize();
		ci.pQueueFamilyIndices = &m_factory->m_gr->getQueueFamilies()[0];
		ci.imageSharingMode = (ci.queueFamilyIndexCount > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		ci.compositeAlpha = compositeAlpha;
		ci.presentMode = presentMode;
		ci.clipped = false;
		ci.oldSwapchain = VK_NULL_HANDLE;

		ANKI_VK_CHECK(vkCreateSwapchainKHR(dev, &ci, nullptr, &m_swapchain));
	}

	// Get images
	{
		U32 count = 0;
		ANKI_VK_CHECK(vkGetSwapchainImagesKHR(dev, m_swapchain, &count, nullptr));
		if(count != kMaxFramesInFlight)
		{
			ANKI_VK_LOGI("Requested a swapchain with %u images but got one with %u", kMaxFramesInFlight, count);
		}

		m_textures.create(getMemoryPool(), count);

		ANKI_VK_LOGI("Created a swapchain. Image count: %u, present mode: %u, size: %ux%u, vsync: %u", count,
					 presentMode, surfaceWidth, surfaceHeight, U32(m_factory->m_vsync));

		Array<VkImage, 64> images;
		ANKI_ASSERT(count <= 64);
		ANKI_VK_CHECK(vkGetSwapchainImagesKHR(dev, m_swapchain, &count, &images[0]));
		for(U32 i = 0; i < count; ++i)
		{
			TextureInitInfo init("SwapchainImg");
			init.m_width = surfaceWidth;
			init.m_height = surfaceHeight;
			init.m_format = Format(surfaceFormat); // anki::Format is compatible with VkFormat
			init.m_usage = TextureUsageBit::kImageComputeWrite | TextureUsageBit::kImageTraceRaysWrite
						   | TextureUsageBit::kFramebufferRead | TextureUsageBit::kFramebufferWrite
						   | TextureUsageBit::kPresent;
			init.m_type = TextureType::k2D;

			TextureImpl* tex =
				newInstance<TextureImpl>(m_factory->m_gr->getMemoryPool(), m_factory->m_gr, init.getName());
			m_textures[i].reset(tex);
			ANKI_CHECK(tex->initExternal(images[i], init));
		}
	}

	return Error::kNone;
}

HeapMemoryPool& MicroSwapchain::getMemoryPool()
{
	return m_factory->m_gr->getMemoryPool();
}

MicroSwapchainPtr SwapchainFactory::newInstance()
{
	// Delete stale swapchains (they are stale because they are probably out of data) and always create a new one
	m_recycler.trimCache();

	// This is useless but call it to avoid assertions
	[[maybe_unused]] MicroSwapchain* dummy = m_recycler.findToReuse();
	ANKI_ASSERT(dummy == nullptr);

	return MicroSwapchainPtr(anki::newInstance<MicroSwapchain>(m_gr->getMemoryPool(), this));
}

void SwapchainFactory::init(GrManagerImpl* manager, Bool vsync)
{
	ANKI_ASSERT(manager);
	m_gr = manager;
	m_vsync = vsync;
	m_recycler.init(&m_gr->getMemoryPool());
}

} // end namespace anki
