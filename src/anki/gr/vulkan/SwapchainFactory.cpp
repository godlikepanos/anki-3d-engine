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

	for(VkFramebuffer& fb : m_framebuffers)
	{
		if(fb)
		{
			vkDestroyFramebuffer(dev, fb, nullptr);
			fb = {};
		}
	}

	for(VkRenderPass& rpass : m_rpasses)
	{
		if(rpass)
		{
			vkDestroyRenderPass(dev, rpass, nullptr);
			rpass = {};
		}
	}

	for(VkImageView& iview : m_imageViews)
	{
		if(iview)
		{
			vkDestroyImageView(dev, iview, nullptr);
			iview = {};
		}
	}

	if(m_swapchain)
	{
		vkDestroySwapchainKHR(dev, m_swapchain, nullptr);
		m_swapchain = {};
		m_images = {};
	}
}

Error MicroSwapchain::initInternal()
{
	const VkDevice dev = m_factory->m_gr->getDevice();

	// Get the surface size
	VkSurfaceCapabilitiesKHR surfaceProperties;
	{
		ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			m_factory->m_gr->getPhysicalDevice(), m_factory->m_gr->getSurface(), &surfaceProperties));

		if(surfaceProperties.currentExtent.width == MAX_U32 || surfaceProperties.currentExtent.height == MAX_U32)
		{
			ANKI_VK_LOGE("Wrong surface size");
			return Error::FUNCTION_FAILED;
		}
		m_surfaceWidth = surfaceProperties.currentExtent.width;
		m_surfaceHeight = surfaceProperties.currentExtent.height;
	}

	// Get the surface format
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
				m_surfaceFormat = formats[formatCount].format;
				colorspace = formats[formatCount].colorSpace;
				break;
			}
		}

		if(m_surfaceFormat == VK_FORMAT_UNDEFINED)
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
		ci.imageFormat = m_surfaceFormat;
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
			m_surfaceWidth,
			m_surfaceHeight,
			U32(m_factory->m_vsync));

		Array<VkImage, MAX_FRAMES_IN_FLIGHT> images;
		ANKI_VK_CHECK(vkGetSwapchainImagesKHR(dev, m_swapchain, &count, &images[0]));
		for(U i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_images[i] = images[i];
			ANKI_ASSERT(images[i]);
		}
	}

	// Create img views
	for(U i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkImageViewCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.flags = 0;
		ci.image = m_images[i];
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = m_surfaceFormat;
		ci.components = {
			VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
		ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ci.subresourceRange.baseMipLevel = 0;
		ci.subresourceRange.levelCount = 1;
		ci.subresourceRange.baseArrayLayer = 0;
		ci.subresourceRange.layerCount = 1;

		ANKI_VK_CHECK(vkCreateImageView(dev, &ci, nullptr, &m_imageViews[i]));
		m_factory->m_gr->trySetVulkanHandleName(
			"DfldImgView", VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, m_imageViews[i]);
	}

	// Create the render passes
	static const Array<VkAttachmentLoadOp, RPASS_COUNT> loadOps = {
		{VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_DONT_CARE}};
	for(U i = 0; i < RPASS_COUNT; ++i)
	{
		const VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription desc = {};
		desc.format = m_surfaceFormat;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = loadOps[i];
		desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = layout;
		desc.finalLayout = layout;

		VkAttachmentReference ref = {0, layout};

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &ref;
		subpass.pDepthStencilAttachment = nullptr;

		VkRenderPassCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		ci.attachmentCount = 1;
		ci.pAttachments = &desc;
		ci.subpassCount = 1;
		ci.pSubpasses = &subpass;

		ANKI_VK_CHECK(vkCreateRenderPass(dev, &ci, nullptr, &m_rpasses[i]));
		m_factory->m_gr->trySetVulkanHandleName(
			"Dfld Rpass", VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, m_rpasses[i]);
	}

	// Create FBs
	for(U i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkFramebufferCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		ci.renderPass = m_rpasses[0]; // Use that, it's compatible
		ci.attachmentCount = 1;
		ci.pAttachments = &m_imageViews[i];
		ci.width = m_surfaceWidth;
		ci.height = m_surfaceHeight;
		ci.layers = 1;

		ANKI_VK_CHECK(vkCreateFramebuffer(dev, &ci, nullptr, &m_framebuffers[i]));
		m_factory->m_gr->trySetVulkanHandleName(
			"Dfld FB", VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, m_framebuffers[i]);
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
