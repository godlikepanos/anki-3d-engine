// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Vulkan/CommandBufferImpl.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/Vulkan/FenceImpl.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Core/ConfigSet.h>
#include <glslang/Public/ShaderLang.h>

namespace anki
{

GrManagerImpl::~GrManagerImpl()
{
	// 1st THING: wait for the present fences because I don't know if waiting on queue will cover this
	for(PerFrame& frame : m_perFrame)
	{
		if(frame.m_presentFence.isCreated())
		{
			frame.m_presentFence->wait();
		}
	}

	// 2nd THING: wait for the GPU
	for(VkQueue& queue : m_queues)
	{
		LockGuard<Mutex> lock(m_globalMtx);
		if(queue)
		{
			vkQueueWaitIdle(queue);
			queue = VK_NULL_HANDLE;
		}
	}

	m_cmdbFactory.destroy();

	// 3rd THING: The destroy everything that has a reference to GrObjects.
	for(PerFrame& frame : m_perFrame)
	{
		frame.m_presentFence.reset(nullptr);
		frame.m_acquireSemaphore.reset(nullptr);
		frame.m_renderSemaphore.reset(nullptr);
	}

	m_crntSwapchain.reset(nullptr);

	// 4th THING: Continue with the rest
	m_gpuMemManager.destroy();

	m_barrierFactory.destroy(); // Destroy before fences
	m_semaphoreFactory.destroy(); // Destroy before fences
	m_swapchainFactory.destroy(); // Destroy before fences

	m_pplineLayoutFactory.destroy();
	m_descrFactory.destroy();

	m_pplineCache.destroy(m_device, m_physicalDevice, getAllocator());

	m_fenceFactory.destroy();

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

	for(QueueType qtype : EnumIterable<QueueType>())
	{
		vkGetDeviceQueue(m_device, m_queueFamilyIndices[qtype], 0, &m_queues[qtype]);
	}

	m_swapchainFactory.init(this, init.m_config->getBool("gr_vsync"));

	m_crntSwapchain = m_swapchainFactory.newInstance();

	ANKI_CHECK(m_pplineCache.init(m_device, m_physicalDevice, init.m_cacheDirectory, *init.m_config, getAllocator()));

	ANKI_CHECK(initMemory(*init.m_config));

	ANKI_CHECK(m_cmdbFactory.init(getAllocator(), m_device, m_queueFamilyIndices));

	for(PerFrame& f : m_perFrame)
	{
		resetFrame(f);
	}

	glslang::InitializeProcess();
	m_fenceFactory.init(getAllocator(), m_device);
	m_semaphoreFactory.init(getAllocator(), m_device);
	m_samplerFactory.init(this);
	m_barrierFactory.init(getAllocator(), m_device);
	m_occlusionQueryFactory.init(getAllocator(), m_device, VK_QUERY_TYPE_OCCLUSION);
	m_timestampQueryFactory.init(getAllocator(), m_device, VK_QUERY_TYPE_TIMESTAMP);

	// Set m_r8g8b8ImagesSupported
	{
		VkImageFormatProperties props = {};
		VkResult res = vkGetPhysicalDeviceImageFormatProperties(
			m_physicalDevice, VK_FORMAT_R8G8B8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &props);

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
		VkResult res = vkGetPhysicalDeviceImageFormatProperties(
			m_physicalDevice, VK_FORMAT_S8_UINT, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, &props);

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
		VkResult res = vkGetPhysicalDeviceImageFormatProperties(
			m_physicalDevice, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, &props);

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

	m_bindlessLimits.m_bindlessTextureCount = init.m_config->getNumberU32("gr_maxBindlessTextures");
	m_bindlessLimits.m_bindlessImageCount = init.m_config->getNumberU32("gr_maxBindlessImages");
	ANKI_CHECK(m_descrFactory.init(getAllocator(), m_device, m_bindlessLimits));
	m_pplineLayoutFactory.init(getAllocator(), m_device);

	return Error::NONE;
}

Error GrManagerImpl::initInstance(const GrManagerInitInfo& init)
{
	// Init VOLK
	//
	ANKI_VK_CHECK(volkInitialize());

	// Create the instance
	//
	const U8 vulkanMinor = init.m_config->getNumberU8("gr_vkminor");
	const U8 vulkanMajor = init.m_config->getNumberU8("gr_vkmajor");

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

	// Validation layers
	static Array<const char*, 1> LAYERS = {"VK_LAYER_KHRONOS_validation"};
	Array<const char*, LAYERS.getSize()> layersToEnable; // Keep it alive in the stack
	if(init.m_config->getBool("gr_validation") || init.m_config->getBool("gr_debugPrintf"))
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
				for(U32 i = 0; i < count; ++i)
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
				for(U32 i = 0; i < layersToEnableCount; ++i)
				{
					ANKI_VK_LOGI("\t%s", layersToEnable[i]);
				}

				ci.enabledLayerCount = layersToEnableCount;
				ci.ppEnabledLayerNames = &layersToEnable[0];
			}
		}
	}

	// Validation features
	DynamicArrayAuto<VkValidationFeatureEnableEXT> enabledValidationFeatures(getAllocator());
	DynamicArrayAuto<VkValidationFeatureDisableEXT> disabledValidationFeatures(getAllocator());
	if(init.m_config->getBool("gr_debugPrintf"))
	{
		enabledValidationFeatures.emplaceBack(VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT);
	}

	if(!init.m_config->getBool("gr_validation"))
	{
		disabledValidationFeatures.emplaceBack(VK_VALIDATION_FEATURE_DISABLE_ALL_EXT);
	}

	VkValidationFeaturesEXT validationFeatures = {};
	if(enabledValidationFeatures.getSize() || disabledValidationFeatures.getSize())
	{
		validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;

		validationFeatures.disabledValidationFeatureCount = disabledValidationFeatures.getSize();
		validationFeatures.enabledValidationFeatureCount = enabledValidationFeatures.getSize();
		validationFeatures.pDisabledValidationFeatures = disabledValidationFeatures.getBegin();
		validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures.getBegin();

		validationFeatures.pNext = ci.pNext;
		ci.pNext = &validationFeatures;
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
		for(U32 i = 0; i < extCount; ++i)
		{
			ANKI_VK_LOGI("\t%s", instExtensionInf[i].extensionName);
		}

		U32 instExtensionCount = 0;

		for(U32 i = 0; i < extCount; ++i)
		{
#if ANKI_OS_LINUX
			if(CString(instExtensionInf[i].extensionName) == VK_KHR_XCB_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_XCB_SURFACE;
				instExtensions[instExtensionCount++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
			}
			else if(CString(instExtensionInf[i].extensionName) == VK_KHR_XLIB_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_XLIB_SURFACE;
				instExtensions[instExtensionCount++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
			}
#elif ANKI_OS_WINDOWS
			if(CString(instExtensionInf[i].extensionName) == VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_WIN32_SURFACE;
				instExtensions[instExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
			}
#elif ANKI_OS_ANDROID
			if(CString(instExtensionInf[i].extensionName) == VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_ANDROID_SURFACE;
				instExtensions[instExtensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
			}
#else
#	error Not implemented
#endif
			else if(CString(instExtensionInf[i].extensionName) == VK_KHR_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_SURFACE;
				instExtensions[instExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
			}
			else if(CString(instExtensionInf[i].extensionName) == VK_EXT_DEBUG_REPORT_EXTENSION_NAME
					&& (init.m_config->getBool("gr_validation") || init.m_config->getBool("gr_debugPrintf")))
			{
				m_extensions |= VulkanExtensions::EXT_DEBUG_REPORT;
				instExtensions[instExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
			}
		}

		if(instExtensionCount)
		{
			ANKI_VK_LOGI("Will enable the following instance extensions:");
			for(U32 i = 0; i < instExtensionCount; ++i)
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

	// Get symbolx
	//
	volkLoadInstance(m_instance);

	// Set debug callbacks
	//
	if(!!(m_extensions & VulkanExtensions::EXT_DEBUG_REPORT))
	{
		VkDebugReportCallbackCreateInfoEXT ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		ci.pfnCallback = debugReportCallbackEXT;
		ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
				   | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

		if(init.m_config->getBool("gr_debugPrintf"))
		{
			ci.flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
		}

		ci.pUserData = this;

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

	m_rtPipelineProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	m_accelerationStructureProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

	m_devProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	m_devProps.pNext = &m_rtPipelineProps;
	m_rtPipelineProps.pNext = &m_accelerationStructureProps;

	vkGetPhysicalDeviceProperties2(m_physicalDevice, &m_devProps);

	// Find vendor
	switch(m_devProps.properties.vendorID)
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
	ANKI_VK_LOGI("GPU is %s. Vendor identified as %s", m_devProps.properties.deviceName,
				 &GPU_VENDOR_STR[m_capabilities.m_gpuVendor][0]);

	// Set limits
	m_capabilities.m_uniformBufferBindOffsetAlignment =
		max<U32>(ANKI_SAFE_ALIGNMENT, U32(m_devProps.properties.limits.minUniformBufferOffsetAlignment));
	m_capabilities.m_uniformBufferMaxRange = m_devProps.properties.limits.maxUniformBufferRange;
	m_capabilities.m_storageBufferBindOffsetAlignment =
		max<U32>(ANKI_SAFE_ALIGNMENT, U32(m_devProps.properties.limits.minStorageBufferOffsetAlignment));
	m_capabilities.m_storageBufferMaxRange = m_devProps.properties.limits.maxStorageBufferRange;
	m_capabilities.m_textureBufferBindOffsetAlignment =
		max<U32>(ANKI_SAFE_ALIGNMENT, U32(m_devProps.properties.limits.minTexelBufferOffsetAlignment));
	m_capabilities.m_textureBufferMaxRange = MAX_U32;

	m_capabilities.m_majorApiVersion = vulkanMajor;
	m_capabilities.m_minorApiVersion = vulkanMinor;

	m_capabilities.m_shaderGroupHandleSize = m_rtPipelineProps.shaderGroupHandleSize;
	m_capabilities.m_sbtRecordAlignment = m_rtPipelineProps.shaderGroupBaseAlignment;

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

	const VkQueueFlags GENERAL_QUEUE_FLAGS = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	for(U32 i = 0; i < count; ++i)
	{
		VkBool32 supportsPresent = false;
		ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresent));

		if(supportsPresent)
		{
			if((queueInfos[i].queueFlags & GENERAL_QUEUE_FLAGS) == GENERAL_QUEUE_FLAGS)
			{
				m_queueFamilyIndices[QueueType::GENERAL] = i;
			}
			else if((queueInfos[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
					&& !(queueInfos[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				// This must be the async compute
				m_queueFamilyIndices[QueueType::COMPUTE] = i;
			}
		}
	}

	if(m_queueFamilyIndices[QueueType::GENERAL] == MAX_U32)
	{
		ANKI_VK_LOGE("Couldn't find a queue family with graphics+compute+transfer+present. "
					 "Something is wrong");
		return Error::FUNCTION_FAILED;
	}

	if(m_queueFamilyIndices[QueueType::COMPUTE] == MAX_U32)
	{
		ANKI_VK_LOGE("Couldn't find an async compute queue");
		return Error::FUNCTION_FAILED;
	}

	const F32 priority = 1.0;
	Array<VkDeviceQueueCreateInfo, U32(QueueType::COUNT)> q = {};
	for(QueueType qtype : EnumIterable<QueueType>())
	{
		q[qtype].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		q[qtype].queueFamilyIndex = m_queueFamilyIndices[qtype];
		q[qtype].queueCount = 1;
		q[qtype].pQueuePriorities = &priority;
	}

	VkDeviceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.queueCreateInfoCount = q.getSize();
	ci.pQueueCreateInfos = &q[0];

	// Extensions
	U32 extCount = 0;
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr);

	DynamicArrayAuto<VkExtensionProperties> extensionInfos(getAllocator()); // Keep it alive in the stack
	DynamicArrayAuto<const char*> extensionsToEnable(getAllocator());
	if(extCount)
	{
		extensionInfos.create(extCount);
		extensionsToEnable.create(extCount);
		U32 extensionsToEnableCount = 0;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, &extensionInfos[0]);

		ANKI_VK_LOGI("Found the following device extensions:");
		for(U32 i = 0; i < extCount; ++i)
		{
			ANKI_VK_LOGI("\t%s", extensionInfos[i].extensionName);
		}

		while(extCount-- != 0)
		{
			const CString extensionName(&extensionInfos[extCount].extensionName[0]);

			if(extensionName == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::KHR_SWAPCHAIN;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_EXT_DEBUG_MARKER_EXTENSION_NAME && init.m_config->getBool("gr_debugMarkers"))
			{
				m_extensions |= VulkanExtensions::EXT_DEBUG_MARKER;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_AMD_SHADER_INFO_EXTENSION_NAME && init.m_config->getBool("core_displayStats"))
			{
				m_extensions |= VulkanExtensions::AMD_SHADER_INFO;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::AMD_RASTERIZATION_ORDER;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME
					&& init.m_config->getBool("gr_rayTracing"))
			{
				m_extensions |= VulkanExtensions::KHR_RAY_TRACING;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
				m_capabilities.m_rayTracingEnabled = true;
			}
			else if(extensionName == VK_KHR_RAY_QUERY_EXTENSION_NAME && init.m_config->getBool("gr_rayTracing"))
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
					&& init.m_config->getBool("gr_rayTracing"))
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
					&& init.m_config->getBool("gr_rayTracing"))
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME && init.m_config->getBool("gr_rayTracing"))
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME
					&& init.m_config->getBool("core_displayStats"))
			{
				m_extensions |= VulkanExtensions::PIPELINE_EXECUTABLE_PROPERTIES;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
					&& init.m_config->getBool("gr_debugPrintf"))
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
		}

		ANKI_VK_LOGI("Will enable the following device extensions:");
		for(U32 i = 0; i < extensionsToEnableCount; ++i)
		{
			ANKI_VK_LOGI("\t%s", extensionsToEnable[i]);
		}

		ci.enabledExtensionCount = extensionsToEnableCount;
		ci.ppEnabledExtensionNames = &extensionsToEnable[0];
	}

	// Enable/disable generic features
	{
		VkPhysicalDeviceFeatures2 devFeatures = {};
		devFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &devFeatures);
		m_devFeatures = devFeatures.features;
		m_devFeatures.robustBufferAccess =
			(init.m_config->getBool("gr_validation") && m_devFeatures.robustBufferAccess) ? true : false;
		ANKI_VK_LOGI("Robust buffer access is %s", (m_devFeatures.robustBufferAccess) ? "enabled" : "disabled");

		ci.pEnabledFeatures = &m_devFeatures;
	}

	// Enable 1.1 features
	{
		m_11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;

		VkPhysicalDeviceFeatures2 features = {};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = &m_11Features;
		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features);

		if(!m_11Features.storageBuffer16BitAccess || !m_11Features.uniformAndStorageBuffer16BitAccess)
		{
			ANKI_VK_LOGE("16bit buffer access is not supported");
			return Error::FUNCTION_FAILED;
		}

		// Disable a few things
		m_11Features.storagePushConstant16 = false; // Because AMD doesn't support it
		m_11Features.protectedMemory = false;
		m_11Features.multiview = false;
		m_11Features.multiviewGeometryShader = false;
		m_11Features.multiviewTessellationShader = false;
		m_11Features.samplerYcbcrConversion = false;

		m_11Features.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &m_11Features;
	}

	// Enable a few 1.2 features
	{
		m_12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

		VkPhysicalDeviceFeatures2 features = {};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = &m_12Features;

		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features);

		// Descriptor indexing
		if(!m_12Features.shaderSampledImageArrayNonUniformIndexing
		   || !m_12Features.shaderStorageImageArrayNonUniformIndexing)
		{
			ANKI_VK_LOGE("Non uniform indexing is not supported by the device");
			return Error::FUNCTION_FAILED;
		}

		if(!m_12Features.descriptorBindingSampledImageUpdateAfterBind
		   || !m_12Features.descriptorBindingStorageImageUpdateAfterBind)
		{
			ANKI_VK_LOGE("Update descriptors after bind is not supported by the device");
			return Error::FUNCTION_FAILED;
		}

		if(!m_12Features.descriptorBindingUpdateUnusedWhilePending)
		{
			ANKI_VK_LOGE("Update descriptors while cmd buffer is pending is not supported by the device");
			return Error::FUNCTION_FAILED;
		}

		// Buffer address
		if(!!(m_extensions & VulkanExtensions::KHR_RAY_TRACING))
		{
			if(!m_12Features.bufferDeviceAddress)
			{
				ANKI_VK_LOGE("Buffer device address is not supported by the device");
				return Error::FUNCTION_FAILED;
			}

			m_12Features.bufferDeviceAddressCaptureReplay =
				m_12Features.bufferDeviceAddressCaptureReplay && init.m_config->getBool("gr_debugMarkers");
			m_12Features.bufferDeviceAddressMultiDevice = false;
		}
		else
		{
			m_12Features.bufferDeviceAddress = false;
			m_12Features.bufferDeviceAddressCaptureReplay = false;
			m_12Features.bufferDeviceAddressMultiDevice = false;
		}

		// Scalar block layout
		if(!m_12Features.scalarBlockLayout)
		{
			ANKI_VK_LOGE("Scalar block layout is not supported by the device");
			return Error::FUNCTION_FAILED;
		}

		// Timeline semaphores
		if(!m_12Features.timelineSemaphore)
		{
			ANKI_VK_LOGE("Timeline semaphores are not supported by the device");
			return Error::FUNCTION_FAILED;
		}

		m_12Features.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &m_12Features;
	}

	// Set RT features
	if(!!(m_extensions & VulkanExtensions::KHR_RAY_TRACING))
	{
		m_rtPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		m_rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
		m_accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

		VkPhysicalDeviceFeatures2 features = {};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = &m_rtPipelineFeatures;
		m_rtPipelineFeatures.pNext = &m_rayQueryFeatures;
		m_rayQueryFeatures.pNext = &m_accelerationStructureFeatures;
		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features);

		if(!m_rtPipelineFeatures.rayTracingPipeline || !m_rayQueryFeatures.rayQuery
		   || !m_accelerationStructureFeatures.accelerationStructure)
		{
			ANKI_VK_LOGE("Ray tracing and ray query are both required");
			return Error::FUNCTION_FAILED;
		}

		// Only enable what's necessary
		m_rtPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplay = false;
		m_rtPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplayMixed = false;
		m_rtPipelineFeatures.rayTraversalPrimitiveCulling = false;
		m_accelerationStructureFeatures.accelerationStructureCaptureReplay = false;
		m_accelerationStructureFeatures.accelerationStructureHostCommands = false;
		m_accelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind = false;

		ANKI_ASSERT(m_accelerationStructureFeatures.pNext == nullptr);
		m_accelerationStructureFeatures.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &m_rtPipelineFeatures;
	}

	// Pipeline features
	if(!!(m_extensions & VulkanExtensions::PIPELINE_EXECUTABLE_PROPERTIES))
	{
		m_pplineExecutablePropertiesFeatures.sType =
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
		m_pplineExecutablePropertiesFeatures.pipelineExecutableInfo = true;

		m_pplineExecutablePropertiesFeatures.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &m_pplineExecutablePropertiesFeatures;
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

	// Print some info
	ANKI_VK_LOGI("Vulkan memory info:");
	for(U32 i = 0; i < m_memoryProperties.memoryHeapCount; ++i)
	{
		ANKI_VK_LOGI("\tHeap %u size %zu", i, m_memoryProperties.memoryHeaps[i].size);
	}
	for(U32 i = 0; i < m_memoryProperties.memoryTypeCount; ++i)
	{
		ANKI_VK_LOGI("\tMem type %u points to heap %u, flags %" ANKI_PRIb32, i,
					 m_memoryProperties.memoryTypes[i].heapIndex,
					 ANKI_FORMAT_U32(m_memoryProperties.memoryTypes[i].propertyFlags));
	}

	m_gpuMemManager.init(m_physicalDevice, m_device, getAllocator(),
						 !!(m_extensions & VulkanExtensions::KHR_RAY_TRACING));

	return Error::NONE;
}

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
void* GrManagerImpl::allocateCallback(void* userData, size_t size, size_t alignment,
									  VkSystemAllocationScope allocationScope)
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

void* GrManagerImpl::reallocateCallback(void* userData, void* original, size_t size, size_t alignment,
										VkSystemAllocationScope allocationScope)
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
	MicroFencePtr fence = m_fenceFactory.newInstance();
	frame.m_acquireSemaphore = m_semaphoreFactory.newInstance(fence, false);

	// Get new image
	uint32_t imageIdx;

	VkResult res = vkAcquireNextImageKHR(m_device, m_crntSwapchain->m_swapchain, UINT64_MAX,
										 frame.m_acquireSemaphore->getHandle(), fence->getHandle(), &imageIdx);

	if(res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ANKI_VK_LOGW("Swapchain is out of date. Will wait for the queue and create a new one");
		for(VkQueue queue : m_queues)
		{
			vkQueueWaitIdle(queue);
		}
		m_crntSwapchain.reset(nullptr);
		m_crntSwapchain = m_swapchainFactory.newInstance();

		// Can't fail a second time
		ANKI_VK_CHECKF(vkAcquireNextImageKHR(m_device, m_crntSwapchain->m_swapchain, UINT64_MAX,
											 frame.m_acquireSemaphore->getHandle(), fence->getHandle(), &imageIdx));
	}
	else
	{
		ANKI_VK_CHECKF(res);
	}

	ANKI_ASSERT(imageIdx < MAX_FRAMES_IN_FLIGHT);
	m_acquiredImageIdx = U8(imageIdx);
	return m_crntSwapchain->m_textures[imageIdx];
}

void GrManagerImpl::endFrame()
{
	ANKI_TRACE_SCOPED_EVENT(VK_PRESENT);

	LockGuard<Mutex> lock(m_globalMtx);

	PerFrame& frame = m_perFrame[m_frame % MAX_FRAMES_IN_FLIGHT];

	// Wait for the fence of N-2 frame
	const U waitFrameIdx = (m_frame + 1) % MAX_FRAMES_IN_FLIGHT;
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
	const U32 idx = m_acquiredImageIdx;
	present.pImageIndices = &idx;
	present.pResults = &res;

	const VkResult res1 = vkQueuePresentKHR(m_queues[frame.m_queueWroteToSwapchainImage], &present);
	if(res1 == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ANKI_VK_LOGW("Swapchain is out of date. Will wait for the queues and create a new one");
		for(VkQueue queue : m_queues)
		{
			vkQueueWaitIdle(queue);
		}
		vkDeviceWaitIdle(m_device);
		m_crntSwapchain.reset(nullptr);
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

void GrManagerImpl::flushCommandBuffer(MicroCommandBufferPtr cmdb, Bool cmdbRenderedToSwapchain,
									   WeakArray<MicroSemaphorePtr> userWaitSemaphores,
									   MicroSemaphorePtr* userSignalSemaphore, Bool wait)
{
	constexpr U32 maxSemaphores = 8;

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	Array<VkSemaphore, maxSemaphores> waitSemaphores;
	submit.pWaitSemaphores = &waitSemaphores[0];
	Array<VkSemaphore, maxSemaphores> signalSemaphores;
	submit.pSignalSemaphores = &signalSemaphores[0];
	Array<VkPipelineStageFlags, maxSemaphores> waitStages;
	submit.pWaitDstStageMask = &waitStages[0];

	// First thing, create a fence
	MicroFencePtr fence = m_fenceFactory.newInstance();

	// Command buffer
	const VkCommandBuffer handle = cmdb->getHandle();
	cmdb->setFence(fence);
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &handle;

	// Handle user semaphores
	Array<U64, maxSemaphores> waitTimelineValues;
	Array<U64, maxSemaphores> signalTimelineValues;

	VkTimelineSemaphoreSubmitInfo timelineInfo = {};
	timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timelineInfo.waitSemaphoreValueCount = userWaitSemaphores.getSize();
	timelineInfo.pWaitSemaphoreValues = &waitTimelineValues[0];
	timelineInfo.signalSemaphoreValueCount = (userSignalSemaphore != nullptr);
	timelineInfo.pSignalSemaphoreValues = &signalTimelineValues[0];
	submit.pNext = &timelineInfo;

	for(MicroSemaphorePtr& userWaitSemaphore : userWaitSemaphores)
	{
		ANKI_ASSERT(userWaitSemaphore.isCreated());
		ANKI_ASSERT(userWaitSemaphore->isTimeline());
		waitSemaphores[submit.waitSemaphoreCount] = userWaitSemaphore->getHandle();

		waitTimelineValues[submit.waitSemaphoreCount] = userWaitSemaphore->getSemaphoreValue();

		// Be a bit conservative
		waitStages[submit.waitSemaphoreCount] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		++submit.waitSemaphoreCount;

		// Refresh the fence because the semaphore can't be recycled until the current submission is done
		userWaitSemaphore->setFence(fence);
	}

	if(userSignalSemaphore)
	{
		*userSignalSemaphore = m_semaphoreFactory.newInstance(fence, true);

		signalSemaphores[submit.signalSemaphoreCount] = (*userSignalSemaphore)->getHandle();

		signalTimelineValues[submit.signalSemaphoreCount] = (*userSignalSemaphore)->getNextSemaphoreValue();

		++submit.signalSemaphoreCount;
	}

	// Protect the class, the queue and other stuff
	LockGuard<Mutex> lock(m_globalMtx);

	// Do some special stuff for the last command buffer
	PerFrame& frame = m_perFrame[m_frame % MAX_FRAMES_IN_FLIGHT];
	if(cmdbRenderedToSwapchain)
	{
		// Wait semaphore
		waitSemaphores[submit.waitSemaphoreCount] = frame.m_acquireSemaphore->getHandle();

		// That depends on how we use the swapchain img. Be a bit conservative
		waitStages[submit.waitSemaphoreCount] =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		++submit.waitSemaphoreCount;

		// Refresh the fence because the semaphore can't be recycled until the current submission is done
		frame.m_acquireSemaphore->setFence(fence);

		// Create the semaphore to signal
		ANKI_ASSERT(!frame.m_renderSemaphore && "Only one begin/end render pass is allowed with the default fb");
		frame.m_renderSemaphore = m_semaphoreFactory.newInstance(fence, false);

		signalSemaphores[submit.signalSemaphoreCount++] = frame.m_renderSemaphore->getHandle();

		// Update the frame fence
		frame.m_presentFence = fence;

		// Update the swapchain's fence
		m_crntSwapchain->setFence(fence);

		frame.m_queueWroteToSwapchainImage = getQueueTypeFromCommandBufferFlags(cmdb->getFlags());
	}

	// Submit
	{
		ANKI_TRACE_SCOPED_EVENT(VK_QUEUE_SUBMIT);
		ANKI_VK_CHECKF(vkQueueSubmit(m_queues[getQueueTypeFromCommandBufferFlags(cmdb->getFlags())], 1, &submit,
									 fence->getHandle()));
	}

	if(wait)
	{
		vkQueueWaitIdle(m_queues[getQueueTypeFromCommandBufferFlags(cmdb->getFlags())]);
	}
}

void GrManagerImpl::finish()
{
	LockGuard<Mutex> lock(m_globalMtx);
	for(VkQueue queue : m_queues)
	{
		vkQueueWaitIdle(queue);
	}
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
			inf.pObjectName = name.cstr();
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

VkBool32 GrManagerImpl::debugReportCallbackEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
											   uint64_t object, size_t location, int32_t messageCode,
											   const char* pLayerPrefix, const char* pMessage, void* pUserData)
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
	if(printPipelineShaderInfoInternal(ppline, name, stages, hash))
	{
		ANKI_VK_LOGE("Ignoring previous errors");
	}
}

Error GrManagerImpl::printPipelineShaderInfoInternal(VkPipeline ppline, CString name, ShaderTypeBit stages,
													 U64 hash) const
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
			ANKI_VK_CHECK(m_pfnGetShaderInfoAMD(m_device, ppline, VkShaderStageFlagBits(convertShaderTypeBit(stage)),
												VK_SHADER_INFO_TYPE_STATISTICS_AMD, &size, &stats));

			str.append(StringAuto(getAllocator())
						   .sprintf("Stage %u: VGRPS %02u, SGRPS %02u ", U32(type), stats.resourceUsage.numUsedVgprs,
									stats.resourceUsage.numUsedSgprs));

			ANKI_CHECK(m_shaderStatsFile.writeText((type != ShaderType::LAST) ? "%u,%u," : "%u,%u\n",
												   stats.resourceUsage.numUsedVgprs, stats.resourceUsage.numUsedSgprs));
		}

		ANKI_VK_LOGI("Pipeline \"%s\" (0x%016" PRIx64 ") stats: %s", name.cstr(), hash, str.cstr());

		// Flush the file just in case
		ANKI_CHECK(m_shaderStatsFile.flush());
	}

	if(!!(m_extensions & VulkanExtensions::PIPELINE_EXECUTABLE_PROPERTIES))
	{
		StringListAuto log(m_alloc);

		VkPipelineInfoKHR pplineInf = {};
		pplineInf.sType = VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR;
		pplineInf.pipeline = ppline;
		U32 executableCount = 0;
		ANKI_VK_CHECK(vkGetPipelineExecutablePropertiesKHR(m_device, &pplineInf, &executableCount, nullptr));
		DynamicArrayAuto<VkPipelineExecutablePropertiesKHR> executableProps(m_alloc, executableCount);
		ANKI_VK_CHECK(
			vkGetPipelineExecutablePropertiesKHR(m_device, &pplineInf, &executableCount, &executableProps[0]));

		log.pushBackSprintf("Pipeline info \"%s\" (0x%016" PRIx64 "): ", name.cstr(), hash);
		for(U32 i = 0; i < executableCount; ++i)
		{
			const VkPipelineExecutablePropertiesKHR& p = executableProps[i];
			log.pushBackSprintf("%s: ", p.description);

			// Get stats
			VkPipelineExecutableInfoKHR exeInf = {};
			exeInf.sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR;
			exeInf.executableIndex = i;
			exeInf.pipeline = ppline;
			U32 statCount = 0;
			vkGetPipelineExecutableStatisticsKHR(m_device, &exeInf, &statCount, nullptr);
			DynamicArrayAuto<VkPipelineExecutableStatisticKHR> stats(m_alloc, statCount);
			vkGetPipelineExecutableStatisticsKHR(m_device, &exeInf, &statCount, &stats[0]);

			for(U32 s = 0; s < statCount; ++s)
			{
				const VkPipelineExecutableStatisticKHR& ss = stats[s];

				switch(ss.format)
				{
				case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR:
					log.pushBackSprintf("%s: %u, ", ss.name, ss.value.b32);
					break;
				case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR:
					log.pushBackSprintf("%s: %" PRId64 ", ", ss.name, ss.value.i64);
					break;
				case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR:
					log.pushBackSprintf("%s: %" PRIu64 ", ", ss.name, ss.value.u64);
					break;
				case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR:
					log.pushBackSprintf("%s: %f, ", ss.name, ss.value.f64);
					break;
				default:
					ANKI_ASSERT(0);
				}
			}

			log.pushBackSprintf("Subgroup size: %u", p.subgroupSize);

			if(i < executableCount - 1)
			{
				log.pushBack(", ");
			}
		}

		StringAuto finalLog(m_alloc);
		log.join("", finalLog);
		ANKI_VK_LOGI("%s", finalLog.cstr());
	}

	return Error::NONE;
}

} // end namespace anki
