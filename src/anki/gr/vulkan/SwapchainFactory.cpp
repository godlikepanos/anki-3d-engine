// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/SwapchainFactory.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

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

	for(TexturePtr& tex : m_textures)
	{
		tex.reset(nullptr);
	}

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
	U surfaceWidth = 0, surfaceHeight = 0;
	{
		ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			m_factory->m_gr->getPhysicalDevice(), m_factory->m_gr->getSurface(), &surfaceProperties));

		if(surfaceProperties.currentExtent.width == MAX_U32 || surfaceProperties.currentExtent.height == MAX_U32)
		{
			ANKI_VK_LOGE("Wrong surface size");
			return Error::FUNCTION_FAILED;
		}
		surfaceWidth = surfaceProperties.currentExtent.width;
		surfaceHeight = surfaceProperties.currentExtent.height;
	}

	// Get the surface format
	VkFormat surfaceFormat = VK_FORMAT_END_RANGE;
	VkColorSpaceKHR colorspace = VK_COLOR_SPACE_MAX_ENUM_KHR;
	{
		uint32_t formatCount;
		ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
			m_factory->m_gr->getPhysicalDevice(), m_factory->m_gr->getSurface(), &formatCount, nullptr));

		DynamicArrayAuto<VkSurfaceFormatKHR> formats(getAllocator());
		formats.create(formatCount);
		ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
			m_factory->m_gr->getPhysicalDevice(), m_factory->m_gr->getSurface(), &formatCount, &formats[0]));

		while(formatCount--)
		{
			if(formats[formatCount].format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				surfaceFormat = formats[formatCount].format;
				colorspace = formats[formatCount].colorSpace;
				break;
			}
		}

		if(surfaceFormat == VK_FORMAT_UNDEFINED)
		{
			ANKI_VK_LOGE("Surface format not found");
			return Error::FUNCTION_FAILED;
		}
	}

	// Chose present mode
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			m_factory->m_gr->getPhysicalDevice(), m_factory->m_gr->getSurface(), &presentModeCount, nullptr);
		presentModeCount = min(presentModeCount, 4u);
		Array<VkPresentModeKHR, 4> presentModes;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			m_factory->m_gr->getPhysicalDevice(), m_factory->m_gr->getSurface(), &presentModeCount, &presentModes[0]);

		if(m_factory->m_vsync)
		{
			presentMode = VK_PRESENT_MODE_FIFO_KHR;
		}
		else
		{
			for(U i = 0; i < presentModeCount; ++i)
			{
				if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				else if(presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
					break;
				}
			}
		}

		if(presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
		{
			ANKI_VK_LOGE("Couldn't find a present mode");
			return Error::FUNCTION_FAILED;
		}
	}

	// Create swapchain
	{
		VkSwapchainCreateInfoKHR ci = {};
		ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		ci.surface = m_factory->m_gr->getSurface();
		ci.minImageCount = MAX_FRAMES_IN_FLIGHT;
		ci.imageFormat = surfaceFormat;
		ci.imageColorSpace = colorspace;
		ci.imageExtent = surfaceProperties.currentExtent;
		ci.imageArrayLayers = 1;
		ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ci.queueFamilyIndexCount = 1;
		U32 idx = m_factory->m_gr->getGraphicsQueueIndex();
		ci.pQueueFamilyIndices = &idx;
		ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		ci.presentMode = presentMode;
		ci.clipped = false;
		ci.oldSwapchain = VK_NULL_HANDLE;

		ANKI_VK_CHECK(vkCreateSwapchainKHR(dev, &ci, nullptr, &m_swapchain));
	}

	// Get images
	{
		uint32_t count = 0;
		ANKI_VK_CHECK(vkGetSwapchainImagesKHR(dev, m_swapchain, &count, nullptr));
		if(count != MAX_FRAMES_IN_FLIGHT)
		{
			ANKI_VK_LOGE("Requested a swapchain with %u images but got one with %u", MAX_FRAMES_IN_FLIGHT, count);
			return Error::FUNCTION_FAILED;
		}

		ANKI_VK_LOGI("Created a swapchain. Image count: %u, present mode: %u, size: %ux%u, vsync: %u",
			count,
			presentMode,
			surfaceWidth,
			surfaceHeight,
			U32(m_factory->m_vsync));

		Array<VkImage, MAX_FRAMES_IN_FLIGHT> images;
		ANKI_VK_CHECK(vkGetSwapchainImagesKHR(dev, m_swapchain, &count, &images[0]));
		for(U i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			TextureInitInfo init("SwapchainImg");
			init.m_width = surfaceWidth;
			init.m_height = surfaceHeight;
			init.m_format = Format::B8G8R8A8_UNORM;
			ANKI_ASSERT(surfaceFormat == VK_FORMAT_B8G8R8A8_UNORM);
			init.m_usage = TextureUsageBit::IMAGE_COMPUTE_WRITE | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE
						   | TextureUsageBit::PRESENT;
			init.m_type = TextureType::_2D;

			TextureImpl* tex =
				m_factory->m_gr->getAllocator().newInstance<TextureImpl>(m_factory->m_gr, init.getName());
			m_textures[i].reset(tex);
			ANKI_CHECK(tex->initExternal(images[i], init));
		}
	}

	return Error::NONE;
}

GrAllocator<U8> MicroSwapchain::getAllocator() const
{
	return m_factory->m_gr->getAllocator();
}

MicroSwapchainPtr SwapchainFactory::newInstance()
{
	MicroSwapchain* out = m_recycler.findToReuse();

	if(out == nullptr)
	{
		// Create a new one
		out = m_gr->getAllocator().newInstance<MicroSwapchain>(this);
	}

	return MicroSwapchainPtr(out);
}

void SwapchainFactory::init(GrManagerImpl* manager, Bool vsync)
{
	ANKI_ASSERT(manager);
	m_gr = manager;
	m_vsync = vsync;
	m_recycler.init(m_gr->getAllocator());
}

} // end namespace anki
