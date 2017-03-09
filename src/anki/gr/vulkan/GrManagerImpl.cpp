// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/GrManager.h>

#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/GrObjectCache.h>

#include <anki/core/Config.h>
#include <glslang/Public/ShaderLang.h>

namespace anki
{

GrManagerImpl::~GrManagerImpl()
{
	// FIRST THING: wait for the GPU
	if(m_queue)
	{
		LockGuard<Mutex> lock(m_globalMtx);
		vkQueueWaitIdle(m_queue);
		m_queue = VK_NULL_HANDLE;
	}

	// SECOND THING: The destroy everything that has a reference to GrObjects.
	for(auto& x : m_backbuffers)
	{
		if(x.m_imageView)
		{
			vkDestroyImageView(m_device, x.m_imageView, nullptr);
			x.m_imageView = VK_NULL_HANDLE;
		}
	}

	for(auto& x : m_perFrame)
	{
		x.m_presentFence.reset(nullptr);
		x.m_acquireSemaphore.reset(nullptr);
		x.m_renderSemaphore.reset(nullptr);

		x.m_cmdbsSubmitted.destroy(getAllocator());
	}

	m_perThread.destroy(getAllocator());

	if(m_samplerCache)
	{
		getAllocator().deleteInstance(m_samplerCache);
	}

	// THIRD THING: Continue with the rest
	m_gpuMemManager.destroy();

	m_semaphores.destroy(); // Destroy before fences
	m_fences.destroy();

	m_pplineLayoutFactory.destroy();
	m_descrFactory.destroy();

	if(m_swapchain)
	{
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	}

	if(m_pplineCache)
	{
		vkDestroyPipelineCache(m_device, m_pplineCache, nullptr);
	}

	if(m_device)
	{
		vkDestroyDevice(m_device, nullptr);
	}

	if(m_surface)
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}

	if(m_instance)
	{
#if ANKI_GR_MANAGER_DEBUG_MEMMORY
		VkAllocationCallbacks* pallocCbs = &m_debugAllocCbs;
#else
		VkAllocationCallbacks* pallocCbs = nullptr;
#endif

		vkDestroyInstance(m_instance, pallocCbs);
	}
}

GrAllocator<U8> GrManagerImpl::getAllocator() const
{
	return m_manager->getAllocator();
}

Error GrManagerImpl::init(const GrManagerInitInfo& init)
{
	Error err = initInternal(init);
	if(err)
	{
		ANKI_VK_LOGE("Vulkan initialization failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	return ErrorCode::NONE;
}

Error GrManagerImpl::initInternal(const GrManagerInitInfo& init)
{
	ANKI_VK_LOGI("Initializing Vulkan backend");
	ANKI_CHECK(initInstance(init));
	ANKI_CHECK(initSurface(init));
	ANKI_CHECK(initDevice(init));
	vkGetDeviceQueue(m_device, m_queueIdx, 0, &m_queue);
	ANKI_CHECK(initSwapchain(init));

	{
		VkPipelineCacheCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vkCreatePipelineCache(m_device, &ci, nullptr, &m_pplineCache);
	}

	ANKI_CHECK(initMemory(*init.m_config));

	for(PerFrame& f : m_perFrame)
	{
		resetFrame(f);
	}

	glslang::InitializeProcess();
	m_fences.init(getAllocator(), m_device);
	m_semaphores.init(getAllocator(), m_device);

	m_queryAlloc.init(getAllocator(), m_device);

	m_samplerCache = getAllocator().newInstance<GrObjectCache>(m_manager);

	// Set m_r8g8b8ImagesSupported
	{
		VkImageFormatProperties props = {};
		VkResult res = vkGetPhysicalDeviceImageFormatProperties(m_physicalDevice,
			VK_FORMAT_R8G8B8_UNORM,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			0,
			&props);

		if(res == VK_ERROR_FORMAT_NOT_SUPPORTED)
		{
			ANKI_VK_LOGI("R8G8B8 Images are not supported. Will workaround this");
			m_r8g8b8ImagesSupported = false;
		}
		else
		{
			ANKI_ASSERT(res == VK_SUCCESS);
			ANKI_VK_LOGI("R8G8B8 Images are supported");
			m_r8g8b8ImagesSupported = true;
		}
	}

	// Set m_s8ImagesSupported
	{
		VkImageFormatProperties props = {};
		VkResult res = vkGetPhysicalDeviceImageFormatProperties(m_physicalDevice,
			VK_FORMAT_S8_UINT,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			0,
			&props);

		if(res == VK_ERROR_FORMAT_NOT_SUPPORTED)
		{
			ANKI_VK_LOGI("S8 Images are not supported. Will workaround this");
			m_s8ImagesSupported = false;
		}
		else
		{
			ANKI_ASSERT(res == VK_SUCCESS);
			ANKI_VK_LOGI("S8 Images are supported");
			m_s8ImagesSupported = true;
		}
	}

	// Set m_d24S8ImagesSupported
	{
		VkImageFormatProperties props = {};
		VkResult res = vkGetPhysicalDeviceImageFormatProperties(m_physicalDevice,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			0,
			&props);

		if(res == VK_ERROR_FORMAT_NOT_SUPPORTED)
		{
			ANKI_VK_LOGI("D24S8 Images are not supported. Will workaround this");
			m_d24S8ImagesSupported = false;
		}
		else
		{
			ANKI_ASSERT(res == VK_SUCCESS);
			ANKI_VK_LOGI("D24S8 Images are supported");
			m_d24S8ImagesSupported = true;
		}
	}

	m_descrFactory.init(getAllocator(), m_device);
	m_pplineLayoutFactory.init(getAllocator(), m_device);

	return ErrorCode::NONE;
}

Error GrManagerImpl::initInstance(const GrManagerInitInfo& init)
{
	// Create the instance
	//
	VkApplicationInfo app = {};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "unamed";
	app.applicationVersion = 1;
	app.pEngineName = "AnKi 3D Engine";
	app.engineVersion = (ANKI_VERSION_MAJOR << 16) | ANKI_VERSION_MINOR;
	app.apiVersion = VK_MAKE_VERSION(1, 0, 3);

	VkInstanceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pApplicationInfo = &app;

	// Layers
	static Array<const char*, 7> LAYERS = {{"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_GOOGLE_unique_objects",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_standard_validation"}};
	Array<const char*, LAYERS.getSize()> layersToEnable; // Keep it alive in the stack
	if(init.m_config->getNumber("debugContext"))
	{
		uint32_t count;
		vkEnumerateInstanceLayerProperties(&count, nullptr);

		if(count)
		{
			DynamicArrayAuto<VkLayerProperties> layerProps(getAllocator());
			layerProps.create(count);

			vkEnumerateInstanceLayerProperties(&count, &layerProps[0]);

			U32 layersToEnableCount = 0;
			for(const char* c : LAYERS)
			{
				for(U i = 0; i < count; ++i)
				{
					if(CString(c) == layerProps[i].layerName)
					{
						layersToEnable[layersToEnableCount++] = c;
						break;
					}
				}
			}

			if(layersToEnableCount)
			{
				ANKI_VK_LOGI("Will enable the following layers:");
				for(U i = 0; i < layersToEnableCount; ++i)
				{
					ANKI_VK_LOGI("\t%s", layersToEnable[i]);
				}

				ci.enabledLayerCount = layersToEnableCount;
				ci.ppEnabledLayerNames = &layersToEnable[0];
			}
		}
	}

	// Extensions
	DynamicArrayAuto<const char*> instExtensions(getAllocator());
	DynamicArrayAuto<VkExtensionProperties> instExtensionInf(getAllocator());
	U32 extCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
	if(extCount)
	{
		instExtensions.create(extCount);
		instExtensionInf.create(extCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &instExtensionInf[0]);
		U instExtensionCount = 0;

		for(U i = 0; i < extCount; ++i)
		{
#if ANKI_OS == ANKI_OS_LINUX
			if(CString(instExtensionInf[i].extensionName) == VK_KHR_XCB_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_XCB_SURFACE;
				instExtensions[instExtensionCount++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
			}
#elif ANKI_OS == ANKI_OS_WINDOWS
			if(CString(instExtensionInf[i].extensionName) == VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_WIN32_SURFACE;
				instExtensions[instExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
			}
#else
#error TODO
#endif
			else if(CString(instExtensionInf[i].extensionName) == VK_KHR_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_SURFACE;
				instExtensions[instExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
			}
		}

		if(instExtensionCount)
		{
			ANKI_VK_LOGI("Will enable the following instance extensions:");
			for(U i = 0; i < instExtensionCount; ++i)
			{
				ANKI_VK_LOGI("\t%s", instExtensions[i]);
			}

			ci.enabledExtensionCount = instExtensionCount;
			ci.ppEnabledExtensionNames = &instExtensions[0];
		}
	}

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
	m_debugAllocCbs = {};
	m_debugAllocCbs.pUserData = this;
	m_debugAllocCbs.pfnAllocation = allocateCallback;
	m_debugAllocCbs.pfnReallocation = reallocateCallback;
	m_debugAllocCbs.pfnFree = freeCallback;

	VkAllocationCallbacks* pallocCbs = &m_debugAllocCbs;
#else
	VkAllocationCallbacks* pallocCbs = nullptr;
#endif

	ANKI_VK_CHECK(vkCreateInstance(&ci, pallocCbs, &m_instance));

	// Create the physical device
	//
	uint32_t count = 0;
	ANKI_VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &count, nullptr));
	ANKI_VK_LOGI("Number of physical devices: %u", count);
	if(count < 1)
	{
		ANKI_VK_LOGE("Wrong number of physical devices");
		return ErrorCode::FUNCTION_FAILED;
	}

	count = 1;
	ANKI_VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &count, &m_physicalDevice));

	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_devProps);

	// Find vendor
	switch(m_devProps.vendorID)
	{
	case 0x13B5:
		m_vendor = GpuVendor::ARM;
		break;
	case 0x10DE:
		m_vendor = GpuVendor::NVIDIA;
		break;
	case 0x1002:
	case 0x1022:
		m_vendor = GpuVendor::AMD;
		break;
	case 0x8086:
		m_vendor = GpuVendor::INTEL;
		break;
	default:
		m_vendor = GpuVendor::UNKNOWN;
	}
	ANKI_VK_LOGI("GPU vendor is %s", &GPU_VENDOR_STR[m_vendor][0]);

	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_devFeatures);

	return ErrorCode::NONE;
}

Error GrManagerImpl::initDevice(const GrManagerInitInfo& init)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, nullptr);
	ANKI_VK_LOGI("Number of queue families: %u", count);

	DynamicArrayAuto<VkQueueFamilyProperties> queueInfos(getAllocator());
	queueInfos.create(count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, &queueInfos[0]);

	uint32_t desiredFamilyIdx = MAX_U32;
	const VkQueueFlags DESITED_QUEUE_FLAGS = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	for(U i = 0; i < count; ++i)
	{
		if((queueInfos[i].queueFlags & DESITED_QUEUE_FLAGS) == DESITED_QUEUE_FLAGS)
		{
			VkBool32 supportsPresent = false;
			ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresent));

			if(supportsPresent)
			{
				desiredFamilyIdx = i;
				break;
			}
		}
	}

	if(desiredFamilyIdx == MAX_U32)
	{
		ANKI_VK_LOGE("Couldn't find a queue family with graphics+compute+transfer+present."
					 "The assumption was wrong. The code needs to be reworked");
		return ErrorCode::FUNCTION_FAILED;
	}

	m_queueIdx = desiredFamilyIdx;

	F32 priority = 1.0;
	VkDeviceQueueCreateInfo q = {};
	q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	q.queueFamilyIndex = desiredFamilyIdx;
	q.queueCount = 1;
	q.pQueuePriorities = &priority;

	VkDeviceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.queueCreateInfoCount = 1;
	ci.pQueueCreateInfos = &q;
	ci.pEnabledFeatures = &m_devFeatures;

	// Extensions
	U32 extCount = 0;
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr);

	DynamicArrayAuto<VkExtensionProperties> extensionInfos(getAllocator()); // Keep it alive in the stack
	DynamicArrayAuto<const char*> extensionsToEnable(getAllocator());
	if(extCount)
	{
		extensionInfos.create(extCount);
		extensionsToEnable.create(extCount);
		U extensionsToEnableCount = 0;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, &extensionInfos[0]);

		while(extCount-- != 0)
		{
			if(CString(&extensionInfos[extCount].extensionName[0]) == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_SWAPCHAIN;
				extensionsToEnable[extensionsToEnableCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			}
			else if(CString(&extensionInfos[extCount].extensionName[0]) == VK_KHR_MAINTENANCE1_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_MAINENANCE1;
				extensionsToEnable[extensionsToEnableCount++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;
			}
			else if(CString(&extensionInfos[extCount].extensionName[0])
				== VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::AMD_NEGATIVE_VIEWPORT_HEIGHT;
				// Don't add it just yet. Can't enable it at the same time with VK_KHR_maintenance1
			}
			else if(CString(extensionInfos[extCount].extensionName) == VK_EXT_DEBUG_MARKER_EXTENSION_NAME
				&& init.m_config->getNumber("debugContext"))
			{
				m_extensions |= VulkanExtensions::EXT_DEBUG_MARKER;
				extensionsToEnable[extensionsToEnableCount++] = VK_EXT_DEBUG_MARKER_EXTENSION_NAME;
			}
		}

		if(!!(m_extensions & VulkanExtensions::KHR_MAINENANCE1))
		{
			m_extensions = m_extensions & ~VulkanExtensions::AMD_NEGATIVE_VIEWPORT_HEIGHT;
		}
		else if(!!(m_extensions & VulkanExtensions::AMD_NEGATIVE_VIEWPORT_HEIGHT))
		{
			extensionsToEnable[extensionsToEnableCount++] = VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME;
		}
		else
		{
			ANKI_VK_LOGE("VK_KHR_maintenance1 or VK_AMD_negative_viewport_height required");
			return ErrorCode::FUNCTION_FAILED;
		}

		ANKI_VK_LOGI("Will enable the following device extensions:");
		for(U i = 0; i < extensionsToEnableCount; ++i)
		{
			ANKI_VK_LOGI("\t%s", extensionsToEnable[i]);
		}

		ci.enabledExtensionCount = extensionsToEnableCount;
		ci.ppEnabledExtensionNames = &extensionsToEnable[0];
	}

	ANKI_VK_CHECK(vkCreateDevice(m_physicalDevice, &ci, nullptr, &m_device));

	// Get debug marker
	if(!!(m_extensions & VulkanExtensions::EXT_DEBUG_MARKER))
	{
		m_pfnDebugMarkerSetObjectNameEXT = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(
			vkGetDeviceProcAddr(m_device, "vkDebugMarkerSetObjectNameEXT"));

		if(!m_pfnDebugMarkerSetObjectNameEXT)
		{
			ANKI_VK_LOGW("VK_EXT_debug_marker is present but vkDebugMarkerSetObjectNameEXT is not there");
		}
	}

	return ErrorCode::NONE;
}

Error GrManagerImpl::initSwapchain(const GrManagerInitInfo& init)
{
	VkSurfaceCapabilitiesKHR surfaceProperties;
	ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceProperties));

	if(surfaceProperties.currentExtent.width == MAX_U32 || surfaceProperties.currentExtent.height == MAX_U32)
	{
		ANKI_VK_LOGE("Wrong surface size");
		return ErrorCode::FUNCTION_FAILED;
	}
	m_surfaceWidth = surfaceProperties.currentExtent.width;
	m_surfaceHeight = surfaceProperties.currentExtent.height;

	uint32_t formatCount;
	ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr));

	DynamicArrayAuto<VkSurfaceFormatKHR> formats(getAllocator());
	formats.create(formatCount);
	ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, &formats[0]));

	VkColorSpaceKHR colorspace = VK_COLOR_SPACE_MAX_ENUM_KHR;
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
		return ErrorCode::FUNCTION_FAILED;
	}

	// Chose present mode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
	presentModeCount = min(presentModeCount, 4u);
	Array<VkPresentModeKHR, 4> presentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, &presentModes[0]);

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
	if(init.m_config->getNumber("vsync"))
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
		return ErrorCode::FUNCTION_FAILED;
	}

	// Create swapchain
	VkSwapchainCreateInfoKHR ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = m_surface;
	ci.minImageCount = MAX_FRAMES_IN_FLIGHT;
	ci.imageFormat = m_surfaceFormat;
	ci.imageColorSpace = colorspace;
	ci.imageExtent = surfaceProperties.currentExtent;
	ci.imageArrayLayers = 1;
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.queueFamilyIndexCount = 1;
	ci.pQueueFamilyIndices = &m_queueIdx;
	ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	ci.presentMode = presentMode;
	ci.clipped = false;
	ci.oldSwapchain = VK_NULL_HANDLE;

	ANKI_VK_CHECK(vkCreateSwapchainKHR(m_device, &ci, nullptr, &m_swapchain));

	// Get images
	uint32_t count = 0;
	ANKI_VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &count, nullptr));
	if(count != MAX_FRAMES_IN_FLIGHT)
	{
		ANKI_VK_LOGE("Requested a swapchain with %u images but got one with %u", MAX_FRAMES_IN_FLIGHT, count);
		return ErrorCode::FUNCTION_FAILED;
	}

	ANKI_VK_LOGI("Created a swapchain. Image count: %u, present mode: %u", count, presentMode);

	Array<VkImage, MAX_FRAMES_IN_FLIGHT> images;
	ANKI_VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &count, &images[0]));
	for(U i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		m_backbuffers[i].m_image = images[i];
		ANKI_ASSERT(images[i]);
	}

	// Create img views
	for(U i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkImageViewCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.flags = 0;
		ci.image = m_backbuffers[i].m_image;
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = m_surfaceFormat;
		ci.components = {
			VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
		ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ci.subresourceRange.baseMipLevel = 0;
		ci.subresourceRange.levelCount = 1;
		ci.subresourceRange.baseArrayLayer = 0;
		ci.subresourceRange.layerCount = 1;

		ANKI_VK_CHECK(vkCreateImageView(m_device, &ci, nullptr, &m_backbuffers[i].m_imageView));
	}

	return ErrorCode::NONE;
}

Error GrManagerImpl::initMemory(const ConfigSet& cfg)
{
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);
	m_gpuMemManager.init(m_physicalDevice, m_device, getAllocator());

	return ErrorCode::NONE;
}

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
void* GrManagerImpl::allocateCallback(
	void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	ANKI_ASSERT(userData);
	ANKI_ASSERT(size);
	ANKI_ASSERT(isPowerOfTwo(alignment));
	ANKI_ASSERT(alignment <= MAX_ALLOC_ALIGNMENT);

	auto alloc = static_cast<GrManagerImpl*>(userData)->getAllocator();

	PtrSize newSize = size + sizeof(AllocHeader);
	AllocHeader* header = static_cast<AllocHeader*>(alloc.getMemoryPool().allocate(newSize, MAX_ALLOC_ALIGNMENT));
	header->m_sig = ALLOC_SIG;
	header->m_size = size;
	++header;

	return static_cast<AllocHeader*>(header);
}

void* GrManagerImpl::reallocateCallback(
	void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	if(original && size == 0)
	{
		freeCallback(userData, original);
		return nullptr;
	}

	void* mem = allocateCallback(userData, size, alignment, allocationScope);
	if(original)
	{
		// Move the data
		AllocHeader* header = static_cast<AllocHeader*>(original);
		--header;
		ANKI_ASSERT(header->m_sig == ALLOC_SIG);
		memcpy(mem, original, header->m_size);
	}

	return mem;
}

void GrManagerImpl::freeCallback(void* userData, void* ptr)
{
	if(ptr)
	{
		ANKI_ASSERT(userData);
		auto alloc = static_cast<GrManagerImpl*>(userData)->getAllocator();

		AllocHeader* header = static_cast<AllocHeader*>(ptr);
		--header;
		ANKI_ASSERT(header->m_sig == ALLOC_SIG);

		alloc.getMemoryPool().free(header);
	}
}
#endif

void GrManagerImpl::beginFrame()
{
	PerFrame& frame = m_perFrame[m_frame % MAX_FRAMES_IN_FLIGHT];

	// Create sync objects
	FencePtr fence = newFence();
	frame.m_acquireSemaphore = m_semaphores.newInstance(fence);

	// Get new image
	uint32_t imageIdx;
	ANKI_TRACE_START_EVENT(VK_ACQUIRE_IMAGE);
	ANKI_VK_CHECKF(vkAcquireNextImageKHR(
		m_device, m_swapchain, UINT64_MAX, frame.m_acquireSemaphore->getHandle(), fence->getHandle(), &imageIdx));
	ANKI_TRACE_STOP_EVENT(VK_ACQUIRE_IMAGE);

	ANKI_ASSERT(imageIdx < MAX_FRAMES_IN_FLIGHT);
	m_crntBackbufferIdx = imageIdx;
}

void GrManagerImpl::endFrame()
{
	LockGuard<Mutex> lock(m_globalMtx);

	PerFrame& frame = m_perFrame[m_frame % MAX_FRAMES_IN_FLIGHT];

	// Wait for the fence of N-2 frame
	U waitFrameIdx = (m_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	PerFrame& waitFrame = m_perFrame[waitFrameIdx];
	if(waitFrame.m_presentFence)
	{
		waitFrame.m_presentFence->wait();
	}

	resetFrame(waitFrame);

	if(!frame.m_renderSemaphore)
	{
		ANKI_VK_LOGW("Nobody draw to the default framebuffer");
	}

	// Present
	VkResult res;
	VkPresentInfoKHR present = {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.waitSemaphoreCount = (frame.m_renderSemaphore) ? 1 : 0;
	present.pWaitSemaphores = (frame.m_renderSemaphore) ? &frame.m_renderSemaphore->getHandle() : nullptr;
	present.swapchainCount = 1;
	present.pSwapchains = &m_swapchain;
	present.pImageIndices = &m_crntBackbufferIdx;
	present.pResults = &res;

	ANKI_VK_CHECKF(vkQueuePresentKHR(m_queue, &present));
	ANKI_VK_CHECKF(res);

	m_descrFactory.endFrame();

	// Finalize
	++m_frame;
}

void GrManagerImpl::resetFrame(PerFrame& frame)
{
	frame.m_presentFence.reset(nullptr);
	frame.m_acquireSemaphore.reset(nullptr);
	frame.m_renderSemaphore.reset(nullptr);

	frame.m_cmdbsSubmitted.destroy(getAllocator());
}

GrManagerImpl::PerThread& GrManagerImpl::getPerThreadCache(ThreadId tid)
{
	PerThread* thread = nullptr;
	LockGuard<SpinLock> lock(m_perThreadMtx);

	// Find or create a record
	auto it = m_perThread.find(tid);
	if(it != m_perThread.getEnd())
	{
		thread = &(*it);
	}
	else
	{
		m_perThread.emplaceBack(getAllocator(), tid);
		it = m_perThread.find(tid);
		thread = &(*it);
	}

	return *thread;
}

VkCommandBuffer GrManagerImpl::newCommandBuffer(ThreadId tid, CommandBufferFlag cmdbFlags)
{
	// Get the per thread cache
	PerThread& thread = getPerThreadCache(tid);

	// Try initialize the recycler
	if(ANKI_UNLIKELY(!thread.m_cmdbs.isCreated()))
	{
		Error err = thread.m_cmdbs.init(getAllocator(), m_device, m_queueIdx);
		if(err)
		{
			ANKI_VK_LOGF("Cannot recover");
		}
	}

	return thread.m_cmdbs.newCommandBuffer(cmdbFlags);
}

void GrManagerImpl::deleteCommandBuffer(VkCommandBuffer cmdb, CommandBufferFlag cmdbFlags, ThreadId tid)
{
	// Get the per thread cache
	PerThread& thread = getPerThreadCache(tid);

	thread.m_cmdbs.deleteCommandBuffer(cmdb, cmdbFlags);
}

void GrManagerImpl::flushCommandBuffer(CommandBufferPtr cmdb, Bool wait)
{
	CommandBufferImpl& impl = *cmdb->m_impl;
	VkCommandBuffer handle = impl.getHandle();

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	FencePtr fence = newFence();

	LockGuard<Mutex> lock(m_globalMtx);

	PerFrame& frame = m_perFrame[m_frame % MAX_FRAMES_IN_FLIGHT];

	// Do some special stuff for the last command buffer
	VkPipelineStageFlags waitFlags;
	if(impl.renderedToDefaultFramebuffer())
	{
		submit.pWaitSemaphores = &frame.m_acquireSemaphore->getHandle();
		waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit.pWaitDstStageMask = &waitFlags;
		submit.waitSemaphoreCount = 1;

		// Create the semaphore to signal
		ANKI_ASSERT(!frame.m_renderSemaphore && "Only one begin/end render pass is allowed with the default fb");
		frame.m_renderSemaphore = m_semaphores.newInstance(fence);

		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &frame.m_renderSemaphore->getHandle();

		frame.m_presentFence = fence;
	}

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &handle;

	frame.m_cmdbsSubmitted.pushBack(getAllocator(), cmdb);

	ANKI_TRACE_START_EVENT(VK_QUEUE_SUBMIT);
	ANKI_VK_CHECKF(vkQueueSubmit(m_queue, 1, &submit, fence->getHandle()));
	ANKI_TRACE_STOP_EVENT(VK_QUEUE_SUBMIT);

	if(wait)
	{
		vkQueueWaitIdle(m_queue);
	}
}

void GrManagerImpl::trySetVulkanHandleName(CString name, VkDebugReportObjectTypeEXT type, U64 handle) const
{
	if(m_pfnDebugMarkerSetObjectNameEXT && name)
	{
		VkDebugMarkerObjectNameInfoEXT inf = {};
		inf.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
		inf.objectType = type;
		inf.pObjectName = name.get();
		inf.object = handle;

		m_pfnDebugMarkerSetObjectNameEXT(m_device, &inf);
	}
}

} // end namespace anki
