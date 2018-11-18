// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/Fence.h>
#include <anki/gr/vulkan/FenceImpl.h>
#include <anki/util/Functions.h>
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

	m_cmdbFactory.destroy();

	// SECOND THING: The destroy everything that has a reference to GrObjects.
	for(auto& x : m_perFrame)
	{
		x.m_presentFence.reset(nullptr);
		x.m_acquireSemaphore.reset(nullptr);
		x.m_renderSemaphore.reset(nullptr);
	}

	m_crntSwapchain.reset(nullptr);

	// THIRD THING: Continue with the rest
	m_gpuMemManager.destroy();

	m_barrierFactory.destroy(); // Destroy before fences
	m_semaphores.destroy(); // Destroy before fences
	m_swapchainFactory.destroy(); // Destroy before fences

	m_pplineLayoutFactory.destroy();
	m_descrFactory.destroy();

	m_pplineCache.destroy(m_device, m_physicalDevice, getAllocator());

	m_fences.destroy();

	m_samplerFactory.destroy();

	if(m_device)
	{
		vkDestroyDevice(m_device, nullptr);
	}

	if(m_surface)
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}

	if(m_debugCallback)
	{
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
			reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT"));

		vkDestroyDebugReportCallbackEXT(m_instance, m_debugCallback, nullptr);
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

	m_vkHandleToName.destroy(getAllocator());
}

Error GrManagerImpl::init(const GrManagerInitInfo& init)
{
	Error err = initInternal(init);
	if(err)
	{
		ANKI_VK_LOGE("Vulkan initialization failed");
		return Error::FUNCTION_FAILED;
	}

	return Error::NONE;
}

Error GrManagerImpl::initInternal(const GrManagerInitInfo& init)
{
	ANKI_VK_LOGI("Initializing Vulkan backend");
	ANKI_CHECK(initInstance(init));
	ANKI_CHECK(initSurface(init));
	ANKI_CHECK(initDevice(init));
	vkGetDeviceQueue(m_device, m_queueIdx, 0, &m_queue);

	m_swapchainFactory.init(this, init.m_config->getNumber("window.vsync"));

	m_crntSwapchain = m_swapchainFactory.newInstance();

	ANKI_CHECK(m_pplineCache.init(m_device, m_physicalDevice, init.m_cacheDirectory, *init.m_config, getAllocator()));

	ANKI_CHECK(initMemory(*init.m_config));

	ANKI_CHECK(m_cmdbFactory.init(getAllocator(), m_device, m_queueIdx));

	for(PerFrame& f : m_perFrame)
	{
		resetFrame(f);
	}

	glslang::InitializeProcess();
	m_fences.init(getAllocator(), m_device);
	m_semaphores.init(getAllocator(), m_device);
	m_samplerFactory.init(this);
	m_barrierFactory.init(getAllocator(), m_device);
	m_queryAlloc.init(getAllocator(), m_device);

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

	return Error::NONE;
}

Error GrManagerImpl::initInstance(const GrManagerInitInfo& init)
{
	// Create the instance
	//
	const U32 vulkanMinor = init.m_config->getNumber("gr.vkminor");
	const U32 vulkanMajor = init.m_config->getNumber("gr.vkmajor");

	VkApplicationInfo app = {};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "unamed";
	app.applicationVersion = 1;
	app.pEngineName = "AnKi 3D Engine";
	app.engineVersion = (ANKI_VERSION_MAJOR << 16) | ANKI_VERSION_MINOR;
	app.apiVersion = VK_MAKE_VERSION(vulkanMajor, vulkanMinor, 0);

	VkInstanceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pApplicationInfo = &app;

	// Layers
	static Array<const char*, 7> LAYERS = {{"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_standard_validation",
		"VK_LAYER_GOOGLE_unique_objects"}};
	Array<const char*, LAYERS.getSize()> layersToEnable; // Keep it alive in the stack
	if(init.m_config->getNumber("window.debugContext"))
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

		ANKI_VK_LOGI("Found the following instance extensions:");
		for(U i = 0; i < extCount; ++i)
		{
			ANKI_VK_LOGI("\t%s", instExtensionInf[i].extensionName);
		}

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
#	error TODO
#endif
			else if(CString(instExtensionInf[i].extensionName) == VK_KHR_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_SURFACE;
				instExtensions[instExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
			}
			else if(CString(instExtensionInf[i].extensionName) == VK_EXT_DEBUG_REPORT_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::EXT_DEBUG_REPORT;
				instExtensions[instExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
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

	// Set debug callbacks
	//
	if(!!(m_extensions & VulkanExtensions::EXT_DEBUG_REPORT))
	{
		VkDebugReportCallbackCreateInfoEXT ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		ci.pfnCallback = debugReportCallbackEXT;
		ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
				   | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		ci.pUserData = this;

		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
			reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT"));
		ANKI_ASSERT(vkCreateDebugReportCallbackEXT);

		vkCreateDebugReportCallbackEXT(m_instance, &ci, nullptr, &m_debugCallback);
	}

	// Create the physical device
	//
	uint32_t count = 0;
	ANKI_VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &count, nullptr));
	ANKI_VK_LOGI("Number of physical devices: %u", count);
	if(count < 1)
	{
		ANKI_VK_LOGE("Wrong number of physical devices");
		return Error::FUNCTION_FAILED;
	}

	count = 1;
	ANKI_VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &count, &m_physicalDevice));

	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_devProps);

	// Find vendor
	switch(m_devProps.vendorID)
	{
	case 0x13B5:
		m_capabilities.m_gpuVendor = GpuVendor::ARM;
		break;
	case 0x10DE:
		m_capabilities.m_gpuVendor = GpuVendor::NVIDIA;
		break;
	case 0x1002:
	case 0x1022:
		m_capabilities.m_gpuVendor = GpuVendor::AMD;
		break;
	case 0x8086:
		m_capabilities.m_gpuVendor = GpuVendor::INTEL;
		break;
	default:
		m_capabilities.m_gpuVendor = GpuVendor::UNKNOWN;
	}
	ANKI_VK_LOGI(
		"GPU is %s. Vendor identified as %s", m_devProps.deviceName, &GPU_VENDOR_STR[m_capabilities.m_gpuVendor][0]);

	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_devFeatures);
	m_devFeatures.robustBufferAccess =
		(init.m_config->getNumber("window.debugContext") && m_devFeatures.robustBufferAccess) ? true : false;
	ANKI_VK_LOGI("Robust buffer access is %s", (m_devFeatures.robustBufferAccess) ? "enabled" : "disabled");

	// Set limits
	m_capabilities.m_uniformBufferBindOffsetAlignment =
		max<U32>(ANKI_SAFE_ALIGNMENT, m_devProps.limits.minUniformBufferOffsetAlignment);
	m_capabilities.m_uniformBufferMaxRange = m_devProps.limits.maxUniformBufferRange;
	m_capabilities.m_storageBufferBindOffsetAlignment =
		max<U32>(ANKI_SAFE_ALIGNMENT, m_devProps.limits.minStorageBufferOffsetAlignment);
	m_capabilities.m_storageBufferMaxRange = m_devProps.limits.maxStorageBufferRange;
	m_capabilities.m_textureBufferBindOffsetAlignment =
		max<U32>(ANKI_SAFE_ALIGNMENT, m_devProps.limits.minTexelBufferOffsetAlignment);
	m_capabilities.m_textureBufferMaxRange = MAX_U32;

	m_capabilities.m_majorApiVersion = vulkanMajor;
	m_capabilities.m_minorApiVersion = vulkanMinor;

	return Error::NONE;
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
		return Error::FUNCTION_FAILED;
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

		ANKI_VK_LOGI("Found the following device extensions:");
		for(U i = 0; i < extCount; ++i)
		{
			ANKI_VK_LOGI("\t%s", extensionInfos[i].extensionName);
		}

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
					&& init.m_config->getNumber("window.debugMarkers"))
			{
				m_extensions |= VulkanExtensions::EXT_DEBUG_MARKER;
				extensionsToEnable[extensionsToEnableCount++] = VK_EXT_DEBUG_MARKER_EXTENSION_NAME;
			}
			else if(CString(extensionInfos[extCount].extensionName) == VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::NV_DEDICATED_ALLOCATION;
				extensionsToEnable[extensionsToEnableCount++] = VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME;
			}
			else if(CString(extensionInfos[extCount].extensionName) == VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::EXT_SHADER_SUBGROUP_BALLOT;
				extensionsToEnable[extensionsToEnableCount++] = VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME;
			}
			else if(CString(extensionInfos[extCount].extensionName) == VK_AMD_SHADER_INFO_EXTENSION_NAME
					&& init.m_config->getNumber("core.displayStats"))
			{
				m_extensions |= VulkanExtensions::AMD_SHADER_INFO;
				extensionsToEnable[extensionsToEnableCount++] = VK_AMD_SHADER_INFO_EXTENSION_NAME;
			}
			else if(CString(extensionInfos[extCount].extensionName) == VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::AMD_RASTERIZATION_ORDER;
				extensionsToEnable[extensionsToEnableCount++] = VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME;
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
			return Error::FUNCTION_FAILED;
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

		m_pfnCmdDebugMarkerBeginEXT =
			reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerBeginEXT"));
		if(!m_pfnCmdDebugMarkerBeginEXT)
		{
			ANKI_VK_LOGW("VK_EXT_debug_marker is present but vkCmdDebugMarkerBeginEXT is not there");
		}

		m_pfnCmdDebugMarkerEndEXT =
			reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerEndEXT"));
		if(!m_pfnCmdDebugMarkerEndEXT)
		{
			ANKI_VK_LOGW("VK_EXT_debug_marker is present but vkCmdDebugMarkerEndEXT is not there");
		}
	}

	// Get VK_AMD_shader_info entry points
	if(!!(m_extensions & VulkanExtensions::AMD_SHADER_INFO))
	{
		m_pfnGetShaderInfoAMD =
			reinterpret_cast<PFN_vkGetShaderInfoAMD>(vkGetDeviceProcAddr(m_device, "vkGetShaderInfoAMD"));
		if(!m_pfnGetShaderInfoAMD)
		{
			ANKI_VK_LOGW("VK_AMD_shader_info is present but vkGetShaderInfoAMD is not there");
		}
	}

	return Error::NONE;
}

Error GrManagerImpl::initMemory(const ConfigSet& cfg)
{
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);
	m_gpuMemManager.init(m_physicalDevice, m_device, getAllocator());

	return Error::NONE;
}

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
void* GrManagerImpl::allocateCallback(
	void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	if(ANKI_UNLIKELY(size == 0))
	{
		return nullptr;
	}

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

TexturePtr GrManagerImpl::acquireNextPresentableTexture()
{
	ANKI_TRACE_SCOPED_EVENT(VK_ACQUIRE_IMAGE);

	LockGuard<Mutex> lock(m_globalMtx);

	PerFrame& frame = m_perFrame[m_frame % MAX_FRAMES_IN_FLIGHT];

	// Create sync objects
	MicroFencePtr fence = newFence();
	frame.m_acquireSemaphore = m_semaphores.newInstance(fence);

	// Get new image
	uint32_t imageIdx;

	VkResult res = vkAcquireNextImageKHR(m_device,
		m_crntSwapchain->m_swapchain,
		UINT64_MAX,
		frame.m_acquireSemaphore->getHandle(),
		fence->getHandle(),
		&imageIdx);

	if(res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ANKI_VK_LOGW("Swapchain is out of date. Will wait for the queue and create a new one");
		vkQueueWaitIdle(m_queue);
		m_crntSwapchain = m_swapchainFactory.newInstance();

		// Can't fail a second time
		ANKI_VK_CHECKF(vkAcquireNextImageKHR(m_device,
			m_crntSwapchain->m_swapchain,
			UINT64_MAX,
			frame.m_acquireSemaphore->getHandle(),
			fence->getHandle(),
			&imageIdx));
	}
	else
	{
		ANKI_VK_CHECKF(res);
	}

	ANKI_ASSERT(imageIdx < MAX_FRAMES_IN_FLIGHT);
	m_acquiredImageIdx = imageIdx;
	return m_crntSwapchain->m_textures[imageIdx];
}

void GrManagerImpl::endFrame()
{
	ANKI_TRACE_SCOPED_EVENT(VK_PRESENT);

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
	present.pSwapchains = &m_crntSwapchain->m_swapchain;
	U32 idx = m_acquiredImageIdx;
	present.pImageIndices = &idx;
	present.pResults = &res;

	VkResult res1 = vkQueuePresentKHR(m_queue, &present);
	if(res1 == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ANKI_VK_LOGW("Swapchain is out of date. Will wait for the queue and create a new one");
		vkQueueWaitIdle(m_queue);
		m_crntSwapchain = m_swapchainFactory.newInstance();
	}
	else
	{
		ANKI_VK_CHECKF(res1);
		ANKI_VK_CHECKF(res);
	}

	m_descrFactory.endFrame();

	// Finalize
	++m_frame;
}

void GrManagerImpl::resetFrame(PerFrame& frame)
{
	frame.m_presentFence.reset(nullptr);
	frame.m_acquireSemaphore.reset(nullptr);
	frame.m_renderSemaphore.reset(nullptr);
}

void GrManagerImpl::flushCommandBuffer(CommandBufferPtr cmdb, FencePtr* outFence, Bool wait)
{
	CommandBufferImpl& impl = static_cast<CommandBufferImpl&>(*cmdb);
	VkCommandBuffer handle = impl.getHandle();

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	MicroFencePtr fence = newFence();

	// Create fence
	if(outFence)
	{
		outFence->reset(getAllocator().newInstance<FenceImpl>(this, "Flush"));
		static_cast<FenceImpl&>(**outFence).m_fence = fence;
	}

	LockGuard<Mutex> lock(m_globalMtx);

	PerFrame& frame = m_perFrame[m_frame % MAX_FRAMES_IN_FLIGHT];

	// Do some special stuff for the last command buffer
	VkPipelineStageFlags waitFlags;
	if(impl.renderedToDefaultFramebuffer())
	{
		submit.pWaitSemaphores = &frame.m_acquireSemaphore->getHandle();
		waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
					| VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; // TODO That depends on how we use the swapchain img
		submit.pWaitDstStageMask = &waitFlags;
		submit.waitSemaphoreCount = 1;

		// Create the semaphore to signal
		ANKI_ASSERT(!frame.m_renderSemaphore && "Only one begin/end render pass is allowed with the default fb");
		frame.m_renderSemaphore = m_semaphores.newInstance(fence);

		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &frame.m_renderSemaphore->getHandle();

		frame.m_presentFence = fence;

		// Update the swapchain's fence
		m_crntSwapchain->setFence(fence);
	}

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &handle;

	impl.setFence(fence);

	{
		ANKI_TRACE_SCOPED_EVENT(VK_QUEUE_SUBMIT);
		ANKI_VK_CHECKF(vkQueueSubmit(m_queue, 1, &submit, fence->getHandle()));
	}

	if(wait)
	{
		vkQueueWaitIdle(m_queue);
	}
}

void GrManagerImpl::finish()
{
	LockGuard<Mutex> lock(m_globalMtx);
	vkQueueWaitIdle(m_queue);
}

void GrManagerImpl::trySetVulkanHandleName(CString name, VkDebugReportObjectTypeEXT type, U64 handle) const
{
	if(name && name.getLength())
	{
		if(m_pfnDebugMarkerSetObjectNameEXT)
		{
			VkDebugMarkerObjectNameInfoEXT inf = {};
			inf.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
			inf.objectType = type;
			inf.pObjectName = name.get();
			inf.object = handle;

			m_pfnDebugMarkerSetObjectNameEXT(m_device, &inf);
		}

		LockGuard<SpinLock> lock(m_vkHandleToNameLock);
		StringAuto newName(getAllocator());
		newName.create(name);
		m_vkHandleToName.emplace(getAllocator(), computeHash(&handle, sizeof(handle)), std::move(newName));
	}
}

StringAuto GrManagerImpl::tryGetVulkanHandleName(U64 handle) const
{
	StringAuto out(getAllocator());

	LockGuard<SpinLock> lock(m_vkHandleToNameLock);

	auto it = m_vkHandleToName.find(computeHash(&handle, sizeof(handle)));
	CString objName;
	if(it != m_vkHandleToName.getEnd())
	{
		objName = it->toCString();
	}
	else
	{
		objName = "Unnamed";
	}

	out.create(objName);

	return out;
}

VkBool32 GrManagerImpl::debugReportCallbackEXT(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData)
{
	// Get the object name
	GrManagerImpl* self = static_cast<GrManagerImpl*>(pUserData);
	StringAuto name = self->tryGetVulkanHandleName(object);

	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		ANKI_VK_LOGE("%s (handle: %s)", pMessage, name.cstr());
	}
	else if(flags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT))
	{
		ANKI_VK_LOGW("%s (handle: %s)", pMessage, name.cstr());
	}
	else
	{
		ANKI_VK_LOGI("%s (handle: %s)", pMessage, name.cstr());
	}

	return false;
}

void GrManagerImpl::printPipelineShaderInfo(VkPipeline ppline, CString name, ShaderTypeBit stages, U64 hash) const
{
	Error err = printPipelineShaderInfoInternal(ppline, name, stages, hash);
	if(err)
	{
		ANKI_VK_LOGE("Ignoring previous errors");
	}
}

Error GrManagerImpl::printPipelineShaderInfoInternal(
	VkPipeline ppline, CString name, ShaderTypeBit stages, U64 hash) const
{
	if(m_pfnGetShaderInfoAMD)
	{
		VkShaderStatisticsInfoAMD stats = {};

		LockGuard<SpinLock> lock(m_shaderStatsFileMtx);

		// Open the file
		if(!m_shaderStatsFile.isOpen())
		{
			ANKI_CHECK(m_shaderStatsFile.open(
				StringAuto(getAllocator()).sprintf("%s/../ppline_stats.csv", m_cacheDir.cstr()).toCString(),
				FileOpenFlag::WRITE));

			ANKI_CHECK(m_shaderStatsFile.writeText("ppline name,hash,"
												   "stage 0 VGPR,stage 0 SGPR,"
												   "stage 1 VGPR,stage 1 SGPR,"
												   "stage 2 VGPR,stage 2 SGPR,"
												   "stage 3 VGPR,stage 3 SGPR,"
												   "stage 4 VGPR,stage 4 SGPR,"
												   "stage 5 VGPR,stage 5 SGPR\n"));
		}

		ANKI_CHECK(m_shaderStatsFile.writeText("%s,0x%" PRIx64 ",", name.cstr(), hash));

		StringAuto str(getAllocator());

		for(ShaderType type = ShaderType::FIRST; type < ShaderType::COUNT; ++type)
		{
			ShaderTypeBit stage = stages & ShaderTypeBit(1 << type);
			if(!stage)
			{
				ANKI_CHECK(m_shaderStatsFile.writeText((type != ShaderType::LAST) ? "0,0," : "0,0\n"));
				continue;
			}

			size_t size = sizeof(stats);
			ANKI_VK_CHECK(m_pfnGetShaderInfoAMD(
				m_device, ppline, convertShaderTypeBit(stage), VK_SHADER_INFO_TYPE_STATISTICS_AMD, &size, &stats));

			str.append(StringAuto(getAllocator())
						   .sprintf("Stage %u: VGRPS %02u, SGRPS %02u ",
							   U32(type),
							   stats.resourceUsage.numUsedVgprs,
							   stats.resourceUsage.numUsedSgprs));

			ANKI_CHECK(m_shaderStatsFile.writeText((type != ShaderType::LAST) ? "%u,%u," : "%u,%u\n",
				stats.resourceUsage.numUsedVgprs,
				stats.resourceUsage.numUsedSgprs));
		}

		ANKI_VK_LOGI("Pipeline \"%s\" (0x%016" PRIx64 ") stats: %s", name.cstr(), hash, str.cstr());

		// Flush the file just in case
		ANKI_CHECK(m_shaderStatsFile.flush());
	}

	return Error::NONE;
}

} // end namespace anki
