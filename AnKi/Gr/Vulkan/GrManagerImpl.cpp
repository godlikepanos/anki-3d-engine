// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Core/App.h>

namespace anki {

BoolCVar g_validationCVar(CVarSubsystem::kGr, "Validation", false, "Enable or not validation");
static BoolCVar g_debugPrintfCVar(CVarSubsystem::kGr, "DebugPrintf", false, "Enable or not debug printf");
BoolCVar g_debugMarkersCVar(CVarSubsystem::kGr, "DebugMarkers", false, "Enable or not debug markers");
BoolCVar g_vsyncCVar(CVarSubsystem::kGr, "Vsync", false, "Enable or not vsync");
static NumericCVar<U8> g_deviceCVar(CVarSubsystem::kGr, "Device", 0, 0, 16, "Choose an available device. Devices are sorted by performance");
static BoolCVar g_rayTracingCVar(CVarSubsystem::kGr, "RayTracing", false, "Try enabling ray tracing");
static BoolCVar g_64bitAtomicsCVar(CVarSubsystem::kGr, "64bitAtomics", true, "Enable or not 64bit atomics");
static BoolCVar g_samplerFilterMinMaxCVar(CVarSubsystem::kGr, "SamplerFilterMinMax", true, "Enable or not min/max sample filtering");
static BoolCVar g_vrsCVar(CVarSubsystem::kGr, "Vrs", false, "Enable or not VRS");
BoolCVar g_meshShadersCVar(CVarSubsystem::kGr, "MeshShaders", false, "Enable or not mesh shaders");
static BoolCVar g_asyncComputeCVar(CVarSubsystem::kGr, "AsyncCompute", true, "Enable or not async compute");
static NumericCVar<U8> g_vkMinorCVar(CVarSubsystem::kGr, "VkMinor", 1, 1, 1, "Vulkan minor version");
static NumericCVar<U8> g_vkMajorCVar(CVarSubsystem::kGr, "VkMajor", 1, 1, 1, "Vulkan major version");
static StringCVar g_vkLayers(CVarSubsystem::kGr, "VkLayers", "", "VK layers to enable. Seperated by :");

// DLSS related
#define ANKI_VK_NVX_BINARY_IMPORT "VK_NVX_binary_import"

GrManagerImpl::~GrManagerImpl()
{
	ANKI_VK_LOGI("Destroying Vulkan backend");

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

	// 3rd THING: The destroy everything that has a reference to GrObjects.
	m_cmdbFactory.destroy();

	for(PerFrame& frame : m_perFrame)
	{
		frame.m_presentFence.reset(nullptr);
		frame.m_acquireSemaphore.reset(nullptr);
		frame.m_renderSemaphore.reset(nullptr);
	}

	m_crntSwapchain.reset(nullptr);

	// 4th THING: Continue with the rest

	m_barrierFactory.destroy(); // Destroy before fences
	m_semaphoreFactory.destroy(); // Destroy before fences
	m_swapchainFactory.destroy(); // Destroy before fences
	m_frameGarbageCollector.destroy();

	m_gpuMemManager.destroy();

	m_pplineLayoutFactory.destroy();
	m_descrFactory.destroy();

	m_pplineCache.destroy();

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

	if(m_debugUtilsMessager)
	{
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugUtilsMessager, nullptr);
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

#if ANKI_PLATFORM_MOBILE
	anki::deleteInstance(GrMemoryPool::getSingleton(), m_globalCreatePipelineMtx);
#endif

	m_cacheDir.destroy();

	GrMemoryPool::freeSingleton();
}

Error GrManagerImpl::init(const GrManagerInitInfo& init)
{
	const Error err = initInternal(init);
	if(err)
	{
		ANKI_VK_LOGE("Vulkan initialization failed");
		return Error::kFunctionFailed;
	}

	return Error::kNone;
}

Error GrManagerImpl::initInternal(const GrManagerInitInfo& init)
{
	ANKI_VK_LOGI("Initializing Vulkan backend");

	GrMemoryPool::allocateSingleton(init.m_allocCallback, init.m_allocCallbackUserData);

	m_cacheDir = init.m_cacheDirectory;

	ANKI_CHECK(initInstance());
	ANKI_CHECK(initSurface());
	ANKI_CHECK(initDevice());

	for(VulkanQueueType qtype : EnumIterable<VulkanQueueType>())
	{
		if(m_queueFamilyIndices[qtype] != kMaxU32)
		{
			vkGetDeviceQueue(m_device, m_queueFamilyIndices[qtype], 0, &m_queues[qtype]);
		}
		else
		{
			m_queues[qtype] = VK_NULL_HANDLE;
		}
	}

	m_swapchainFactory.init(g_vsyncCVar.get());

	m_crntSwapchain = m_swapchainFactory.newInstance();

	ANKI_CHECK(m_pplineCache.init(init.m_cacheDirectory));

	ANKI_CHECK(initMemory());

	m_cmdbFactory.init(m_queueFamilyIndices);

	for(PerFrame& f : m_perFrame)
	{
		resetFrame(f);
	}

	m_occlusionQueryFactory.init(VK_QUERY_TYPE_OCCLUSION);
	m_timestampQueryFactory.init(VK_QUERY_TYPE_TIMESTAMP);

	// See if unaligned formats are supported
	{
		m_capabilities.m_unalignedBbpTextureFormats = true;

		VkImageFormatProperties props = {};
		VkResult res = vkGetPhysicalDeviceImageFormatProperties(m_physicalDevice, VK_FORMAT_R8G8B8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
																VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &props);
		if(res == VK_ERROR_FORMAT_NOT_SUPPORTED)
		{
			m_capabilities.m_unalignedBbpTextureFormats = false;
		}

		res = vkGetPhysicalDeviceImageFormatProperties(m_physicalDevice, VK_FORMAT_R16G16B16_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
													   VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &props);
		if(res == VK_ERROR_FORMAT_NOT_SUPPORTED)
		{
			m_capabilities.m_unalignedBbpTextureFormats = false;
		}

		res = vkGetPhysicalDeviceImageFormatProperties(m_physicalDevice, VK_FORMAT_R32G32B32_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
													   VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &props);
		if(res == VK_ERROR_FORMAT_NOT_SUPPORTED)
		{
			m_capabilities.m_unalignedBbpTextureFormats = false;
		}

		if(!m_capabilities.m_unalignedBbpTextureFormats)
		{
			ANKI_VK_LOGV("R8G8B8, R16G16B16 and R32G32B32 image formats are not supported");
		}
	}

	ANKI_CHECK(m_descrFactory.init(kMaxBindlessTextures, kMaxBindlessReadonlyTextureBuffers));

	m_frameGarbageCollector.init();

	return Error::kNone;
}

Error GrManagerImpl::initInstance()
{
	// Init VOLK
	//
	ANKI_VK_CHECK(volkInitialize());

	// Create the instance
	//
	const U8 vulkanMinor = g_vkMinorCVar.get();
	const U8 vulkanMajor = g_vkMajorCVar.get();

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

	// Instance layers
	GrDynamicArray<const char*> layersToEnable;
	GrList<GrString> layersToEnableStrings;
	{
		U32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		if(layerCount)
		{
			GrDynamicArray<VkLayerProperties> layerProps;
			layerProps.resize(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, &layerProps[0]);

			ANKI_VK_LOGV("Found the following instance layers:");
			for(const VkLayerProperties& layer : layerProps)
			{
				ANKI_VK_LOGV("\t%s", layer.layerName);
				CString layerName = layer.layerName;

				Bool enableLayer = (g_validationCVar.get() || g_debugMarkersCVar.get()) && layerName == "VK_LAYER_KHRONOS_validation";
				enableLayer = enableLayer || (!g_vkLayers.get().isEmpty() && g_vkLayers.get().find(layerName) != CString::kNpos);

				if(enableLayer)
				{
					layersToEnableStrings.emplaceBack(layer.layerName);
					layersToEnable.emplaceBack(layersToEnableStrings.getBack().cstr());
				}
			}
		}

		if(layersToEnable.getSize())
		{
			ANKI_VK_LOGI("Will enable the following instance layers:");
			for(const char* name : layersToEnable)
			{
				ANKI_VK_LOGI("\t%s", name);
			}

			ci.enabledLayerCount = layersToEnable.getSize();
			ci.ppEnabledLayerNames = &layersToEnable[0];
		}
	}

	// Validation features
	GrDynamicArray<VkValidationFeatureEnableEXT> enabledValidationFeatures;
	GrDynamicArray<VkValidationFeatureDisableEXT> disabledValidationFeatures;
	if(g_debugPrintfCVar.get())
	{
		enabledValidationFeatures.emplaceBack(VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT);
	}

	if(g_debugPrintfCVar.get() && !g_validationCVar.get())
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
	GrDynamicArray<const char*> instExtensions;
	GrDynamicArray<VkExtensionProperties> instExtensionInf;
	U32 extCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
	if(extCount)
	{
		instExtensions.resize(extCount);
		instExtensionInf.resize(extCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &instExtensionInf[0]);

		ANKI_VK_LOGV("Found the following instance extensions:");
		for(U32 i = 0; i < extCount; ++i)
		{
			ANKI_VK_LOGV("\t%s", instExtensionInf[i].extensionName);
		}

		U32 instExtensionCount = 0;

		for(U32 i = 0; i < extCount; ++i)
		{
			const CString extensionName = instExtensionInf[i].extensionName;

#if ANKI_WINDOWING_SYSTEM_HEADLESS
			if(extensionName == VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kEXT_headless_surface;
				instExtensions[instExtensionCount++] = VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME;
			}
#elif ANKI_OS_LINUX
			if(extensionName == VK_KHR_XCB_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_xcb_surface;
				instExtensions[instExtensionCount++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
			}
			else if(extensionName == VK_KHR_XLIB_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_xlib_surface;
				instExtensions[instExtensionCount++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
			}
#elif ANKI_OS_WINDOWS
			if(extensionName == VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_win32_surface;
				instExtensions[instExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
			}
#elif ANKI_OS_ANDROID
			if(extensionName == VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_android_surface;
				instExtensions[instExtensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
			}
#else
#	error Not implemented
#endif
			else if(extensionName == VK_KHR_SURFACE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_surface;
				instExtensions[instExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
			}
			else if(extensionName == VK_EXT_DEBUG_UTILS_EXTENSION_NAME
					&& (g_debugMarkersCVar.get() || g_validationCVar.get() || g_debugPrintfCVar.get()))
			{
				m_extensions |= VulkanExtensions::kEXT_debug_utils;
				instExtensions[instExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			}
		}

		if(!(m_extensions
			 & (VulkanExtensions::kEXT_headless_surface | VulkanExtensions::kKHR_xcb_surface | VulkanExtensions::kKHR_xlib_surface
				| VulkanExtensions::kKHR_win32_surface | VulkanExtensions::kKHR_android_surface)))
		{
			ANKI_VK_LOGE("Couldn't find suitable surface extension");
			return Error::kFunctionFailed;
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
	if(!!(m_extensions & VulkanExtensions::kEXT_debug_utils))
	{
		VkDebugUtilsMessengerCreateInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
							   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
						   | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		info.pfnUserCallback = debugReportCallbackEXT;
		info.pUserData = this;

		vkCreateDebugUtilsMessengerEXT(m_instance, &info, nullptr, &m_debugUtilsMessager);
	}

	// Create the physical device
	//
	{
		uint32_t count = 0;
		ANKI_VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &count, nullptr));
		if(count < 1)
		{
			ANKI_VK_LOGE("Wrong number of physical devices");
			return Error::kFunctionFailed;
		}

		GrDynamicArray<VkPhysicalDevice> physicalDevices;
		physicalDevices.resize(count);
		ANKI_VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &count, &physicalDevices[0]));

		class Dev
		{
		public:
			VkPhysicalDevice m_pdev;
			VkPhysicalDeviceProperties2 m_vkProps;
		};

		GrDynamicArray<Dev> devs;
		devs.resize(count);
		for(U32 devIdx = 0; devIdx < count; ++devIdx)
		{
			devs[devIdx].m_pdev = physicalDevices[devIdx];
			devs[devIdx].m_vkProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			vkGetPhysicalDeviceProperties2(physicalDevices[devIdx], &devs[devIdx].m_vkProps);
		}

		// Sort the devices with the most powerful first
		std::sort(devs.getBegin(), devs.getEnd(), [](const Dev& a, const Dev& b) {
			if(a.m_vkProps.properties.deviceType != b.m_vkProps.properties.deviceType)
			{
				auto findDeviceTypeWeight = [](VkPhysicalDeviceType type) {
					switch(type)
					{
					case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
						return 1.0;
					case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
						return 2.0;
					default:
						return 0.0;
					}
				};

				// Put descrete GPUs first
				return findDeviceTypeWeight(a.m_vkProps.properties.deviceType) > findDeviceTypeWeight(b.m_vkProps.properties.deviceType);
			}
			else
			{
				return a.m_vkProps.properties.apiVersion >= b.m_vkProps.properties.apiVersion;
			}
		});

		const U32 chosenPhysDevIdx = min<U32>(g_deviceCVar.get(), devs.getSize() - 1);

		ANKI_VK_LOGI("Physical devices:");
		for(U32 devIdx = 0; devIdx < count; ++devIdx)
		{
			ANKI_VK_LOGI((devIdx == chosenPhysDevIdx) ? "\t(Selected) %s" : "\t%s", devs[devIdx].m_vkProps.properties.deviceName);
		}

		m_capabilities.m_discreteGpu = devs[chosenPhysDevIdx].m_vkProps.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		m_physicalDevice = devs[chosenPhysDevIdx].m_pdev;
	}

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
		m_capabilities.m_gpuVendor = GpuVendor::kArm;
		m_capabilities.m_minSubgroupSize = 16;
		m_capabilities.m_maxSubgroupSize = 16;
		break;
	case 0x10DE:
		m_capabilities.m_gpuVendor = GpuVendor::kNvidia;
		m_capabilities.m_minSubgroupSize = 32;
		m_capabilities.m_maxSubgroupSize = 32;
		break;
	case 0x1002:
	case 0x1022:
		m_capabilities.m_gpuVendor = GpuVendor::kAMD;
		m_capabilities.m_minSubgroupSize = 32;
		m_capabilities.m_maxSubgroupSize = 64;
		break;
	case 0x8086:
		m_capabilities.m_gpuVendor = GpuVendor::kIntel;
		m_capabilities.m_minSubgroupSize = 8;
		m_capabilities.m_maxSubgroupSize = 32;
		break;
	case 0x5143:
		m_capabilities.m_gpuVendor = GpuVendor::kQualcomm;
		m_capabilities.m_minSubgroupSize = 64;
		m_capabilities.m_maxSubgroupSize = 128;
		break;
	default:
		m_capabilities.m_gpuVendor = GpuVendor::kUnknown;
		// Choose something really low
		m_capabilities.m_minSubgroupSize = 8;
		m_capabilities.m_maxSubgroupSize = 8;
	}
	ANKI_VK_LOGI("GPU is %s. Vendor identified as %s", m_devProps.properties.deviceName, &kGPUVendorStrings[m_capabilities.m_gpuVendor][0]);

	// Set limits
	m_capabilities.m_constantBufferBindOffsetAlignment =
		max<U32>(ANKI_SAFE_ALIGNMENT, U32(m_devProps.properties.limits.minUniformBufferOffsetAlignment));
	m_capabilities.m_constantBufferMaxRange = m_devProps.properties.limits.maxUniformBufferRange;
	m_capabilities.m_uavBufferBindOffsetAlignment = max<U32>(ANKI_SAFE_ALIGNMENT, U32(m_devProps.properties.limits.minStorageBufferOffsetAlignment));
	m_capabilities.m_uavBufferMaxRange = m_devProps.properties.limits.maxStorageBufferRange;
	m_capabilities.m_textureBufferBindOffsetAlignment =
		max<U32>(ANKI_SAFE_ALIGNMENT, U32(m_devProps.properties.limits.minTexelBufferOffsetAlignment));
	m_capabilities.m_textureBufferMaxRange = kMaxU32;
	m_capabilities.m_computeSharedMemorySize = m_devProps.properties.limits.maxComputeSharedMemorySize;
	m_capabilities.m_maxDrawIndirectCount = m_devProps.properties.limits.maxDrawIndirectCount;

	m_capabilities.m_majorApiVersion = vulkanMajor;
	m_capabilities.m_minorApiVersion = vulkanMinor;

	m_capabilities.m_shaderGroupHandleSize = m_rtPipelineProps.shaderGroupHandleSize;
	m_capabilities.m_sbtRecordAlignment = m_rtPipelineProps.shaderGroupBaseAlignment;

#if ANKI_PLATFORM_MOBILE
	if(m_capabilities.m_gpuVendor == GpuVendor::kQualcomm)
	{
		// Calling vkCreateGraphicsPipeline from multiple threads crashes qualcomm's compiler
		ANKI_VK_LOGI("Enabling workaround for vkCreateGraphicsPipeline crashing when called from multiple threads");
		m_globalCreatePipelineMtx = anki::newInstance<Mutex>(GrMemoryPool::getSingleton());
	}
#endif

	// DLSS checks
	m_capabilities.m_dlss = ANKI_DLSS && m_capabilities.m_gpuVendor == GpuVendor::kNvidia;

	return Error::kNone;
}

Error GrManagerImpl::initDevice()
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, nullptr);
	ANKI_VK_LOGI("Number of queue families: %u", count);

	GrDynamicArray<VkQueueFamilyProperties> queueInfos;
	queueInfos.resize(count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, &queueInfos[0]);

	const VkQueueFlags GENERAL_QUEUE_FLAGS = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
	for(U32 i = 0; i < count; ++i)
	{
		VkBool32 supportsPresent = false;
		ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresent));

		if(supportsPresent)
		{
			if((queueInfos[i].queueFlags & GENERAL_QUEUE_FLAGS) == GENERAL_QUEUE_FLAGS)
			{
				m_queueFamilyIndices[VulkanQueueType::kGeneral] = i;
			}
			else if((queueInfos[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && !(queueInfos[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				// This must be the async compute
				m_queueFamilyIndices[VulkanQueueType::kCompute] = i;
			}
		}
	}

	if(m_queueFamilyIndices[VulkanQueueType::kGeneral] == kMaxU32)
	{
		ANKI_VK_LOGE("Couldn't find a queue family with graphics+compute+transfer+present. "
					 "Something is wrong");
		return Error::kFunctionFailed;
	}

	if(!g_asyncComputeCVar.get())
	{
		m_queueFamilyIndices[VulkanQueueType::kCompute] = kMaxU32;
	}

	if(m_queueFamilyIndices[VulkanQueueType::kCompute] == kMaxU32)
	{
		ANKI_VK_LOGW("Couldn't find an async compute queue. Will try to use the general queue instead");
	}
	else
	{
		ANKI_VK_LOGI("Async compute is enabled");
	}

	const F32 priority = 1.0f;
	Array<VkDeviceQueueCreateInfo, U32(VulkanQueueType::kCount)> q = {};

	VkDeviceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.pQueueCreateInfos = &q[0];

	for(VulkanQueueType qtype : EnumIterable<VulkanQueueType>())
	{
		if(m_queueFamilyIndices[qtype] != kMaxU32)
		{
			q[qtype].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			q[qtype].queueFamilyIndex = m_queueFamilyIndices[qtype];
			q[qtype].queueCount = 1;
			q[qtype].pQueuePriorities = &priority;

			++ci.queueCreateInfoCount;
		}
	}

	// Extensions
	U32 extCount = 0;
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr);

	GrDynamicArray<VkExtensionProperties> extensionInfos; // Keep it alive in the stack
	GrDynamicArray<const char*> extensionsToEnable;
	if(extCount)
	{
		extensionInfos.resize(extCount);
		extensionsToEnable.resize(extCount);
		U32 extensionsToEnableCount = 0;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, &extensionInfos[0]);

		ANKI_VK_LOGV("Found the following device extensions:");
		for(U32 i = 0; i < extCount; ++i)
		{
			ANKI_VK_LOGV("\t%s", extensionInfos[i].extensionName);
		}

		while(extCount-- != 0)
		{
			const CString extensionName(&extensionInfos[extCount].extensionName[0]);

			if(extensionName == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_swapchain;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kAMD_rasterization_order;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME && g_rayTracingCVar.get())
			{
				m_extensions |= VulkanExtensions::kKHR_ray_tracing;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
				m_capabilities.m_rayTracingEnabled = true;
			}
			else if(extensionName == VK_KHR_RAY_QUERY_EXTENSION_NAME && g_rayTracingCVar.get())
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME && g_rayTracingCVar.get())
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME && g_rayTracingCVar.get())
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME && g_rayTracingCVar.get())
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME && g_displayStatsCVar.get() > 1)
			{
				m_extensions |= VulkanExtensions::kKHR_pipeline_executable_properties;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME && g_debugPrintfCVar.get())
			{
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kEXT_descriptor_indexing;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_buffer_device_address;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kEXT_scalar_block_layout;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_timeline_semaphore;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_shader_float16_int8;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME && g_64bitAtomicsCVar.get())
			{
				m_extensions |= VulkanExtensions::kKHR_shader_atomic_int64;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_SPIRV_1_4_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_spirv_1_4;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_shader_float_controls;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME && g_samplerFilterMinMaxCVar.get())
			{
				m_extensions |= VulkanExtensions::kKHR_sampler_filter_min_max;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_create_renderpass_2;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME && g_vrsCVar.get())
			{
				m_extensions |= VulkanExtensions::kKHR_fragment_shading_rate;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kEXT_astc_decode_mode;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kEXT_texture_compression_astc_hdr;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(m_capabilities.m_dlss && extensionName == VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_push_descriptor;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(m_capabilities.m_dlss && extensionName == ANKI_VK_NVX_BINARY_IMPORT)
			{
				m_extensions |= VulkanExtensions::kNVX_binary_import;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(m_capabilities.m_dlss && extensionName == VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kNVX_image_view_handle;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_MAINTENANCE_4_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_maintenance_4;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)
			{
				m_extensions |= VulkanExtensions::kKHR_draw_indirect_count;
				extensionsToEnable[extensionsToEnableCount++] = extensionName.cstr();
			}
			else if(extensionName == VK_EXT_MESH_SHADER_EXTENSION_NAME && g_meshShadersCVar.get())
			{
				m_extensions |= VulkanExtensions::kEXT_mesh_shader;
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
	VkPhysicalDeviceFeatures devFeatures = {};
	{
		VkPhysicalDeviceFeatures2 devFeatures2 = {};
		devFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &devFeatures2);
		devFeatures = devFeatures2.features;
		devFeatures.robustBufferAccess = (g_validationCVar.get() && devFeatures.robustBufferAccess) ? true : false;
		ANKI_VK_LOGI("Robust buffer access is %s", (devFeatures.robustBufferAccess) ? "enabled" : "disabled");

		ci.pEnabledFeatures = &devFeatures;
	}

#if ANKI_PLATFORM_MOBILE
	if(!(m_extensions & VulkanExtensions::kEXT_texture_compression_astc_hdr))
	{
		ANKI_VK_LOGE(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME " is not supported");
		return Error::kFunctionFailed;
	}
#endif

	if(!(m_extensions & VulkanExtensions::kKHR_create_renderpass_2))
	{
		ANKI_VK_LOGE(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME " is not supported");
		return Error::kFunctionFailed;
	}

	if(!!(m_extensions & VulkanExtensions::kKHR_sampler_filter_min_max))
	{
		m_capabilities.m_samplingFilterMinMax = true;
	}
	else
	{
		m_capabilities.m_samplingFilterMinMax = false;
		ANKI_VK_LOGI(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME " is not supported or disabled");
	}

	// Descriptor indexing
	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {};
	if(!(m_extensions & VulkanExtensions::kEXT_descriptor_indexing))
	{
		ANKI_VK_LOGE(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME " is not supported");
		return Error::kFunctionFailed;
	}
	else
	{
		descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
		getPhysicalDevicaFeatures2(descriptorIndexingFeatures);

		if(!descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing
		   || !descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing)
		{
			ANKI_VK_LOGE("Non uniform indexing is not supported by the device");
			return Error::kFunctionFailed;
		}

		if(!descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind
		   || !descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind)
		{
			ANKI_VK_LOGE("Update descriptors after bind is not supported by the device");
			return Error::kFunctionFailed;
		}

		if(!descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending)
		{
			ANKI_VK_LOGE("Update descriptors while cmd buffer is pending is not supported by the device");
			return Error::kFunctionFailed;
		}

		descriptorIndexingFeatures.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &descriptorIndexingFeatures;
	}

	// Buffer address
	VkPhysicalDeviceBufferDeviceAddressFeaturesKHR deviceBufferFeatures = {};
	if(!(m_extensions & VulkanExtensions::kKHR_buffer_device_address))
	{
		ANKI_VK_LOGW(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME " is not supported");
	}
	else
	{
		deviceBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
		getPhysicalDevicaFeatures2(deviceBufferFeatures);

		deviceBufferFeatures.bufferDeviceAddressCaptureReplay = deviceBufferFeatures.bufferDeviceAddressCaptureReplay && g_debugMarkersCVar.get();
		deviceBufferFeatures.bufferDeviceAddressMultiDevice = false;

		deviceBufferFeatures.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &deviceBufferFeatures;
	}

	// Scalar block layout
	VkPhysicalDeviceScalarBlockLayoutFeaturesEXT scalarBlockLayoutFeatures = {};
	if(!(m_extensions & VulkanExtensions::kEXT_scalar_block_layout))
	{
		ANKI_VK_LOGE(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME " is not supported");
		return Error::kFunctionFailed;
	}
	else
	{
		scalarBlockLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT;
		getPhysicalDevicaFeatures2(scalarBlockLayoutFeatures);

		if(!scalarBlockLayoutFeatures.scalarBlockLayout)
		{
			ANKI_VK_LOGE("Scalar block layout is not supported by the device");
			return Error::kFunctionFailed;
		}

		scalarBlockLayoutFeatures.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &scalarBlockLayoutFeatures;
	}

	// Timeline semaphore
	VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaphoreFeatures = {};
	if(!(m_extensions & VulkanExtensions::kKHR_timeline_semaphore))
	{
		ANKI_VK_LOGE(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME " is not supported");
		return Error::kFunctionFailed;
	}
	else
	{
		timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;
		getPhysicalDevicaFeatures2(timelineSemaphoreFeatures);

		if(!timelineSemaphoreFeatures.timelineSemaphore)
		{
			ANKI_VK_LOGE("Timeline semaphores are not supported by the device");
			return Error::kFunctionFailed;
		}

		timelineSemaphoreFeatures.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &timelineSemaphoreFeatures;
	}

	// Set RT features
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures = {};
	VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {};
	if(!!(m_extensions & VulkanExtensions::kKHR_ray_tracing))
	{
		rtPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
		accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

		VkPhysicalDeviceFeatures2 features = {};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = &rtPipelineFeatures;
		rtPipelineFeatures.pNext = &rayQueryFeatures;
		rayQueryFeatures.pNext = &accelerationStructureFeatures;
		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features);

		if(!rtPipelineFeatures.rayTracingPipeline || !rayQueryFeatures.rayQuery || !accelerationStructureFeatures.accelerationStructure)
		{
			ANKI_VK_LOGE("Ray tracing and ray query are both required");
			return Error::kFunctionFailed;
		}

		// Only enable what's necessary
		rtPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplay = false;
		rtPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplayMixed = false;
		rtPipelineFeatures.rayTraversalPrimitiveCulling = false;
		accelerationStructureFeatures.accelerationStructureCaptureReplay = false;
		accelerationStructureFeatures.accelerationStructureHostCommands = false;
		accelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind = false;

		ANKI_ASSERT(accelerationStructureFeatures.pNext == nullptr);
		accelerationStructureFeatures.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &rtPipelineFeatures;

		// Get some more stuff
		VkPhysicalDeviceAccelerationStructurePropertiesKHR props = {};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
		getPhysicalDeviceProperties2(props);
		m_capabilities.m_accelerationStructureBuildScratchOffsetAlignment = props.minAccelerationStructureScratchOffsetAlignment;
	}

	// Pipeline features
	VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pplineExecutablePropertiesFeatures = {};
	if(!!(m_extensions & VulkanExtensions::kKHR_pipeline_executable_properties))
	{
		pplineExecutablePropertiesFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
		pplineExecutablePropertiesFeatures.pipelineExecutableInfo = true;

		pplineExecutablePropertiesFeatures.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &pplineExecutablePropertiesFeatures;
	}

	// F16 I8
	VkPhysicalDeviceShaderFloat16Int8FeaturesKHR float16Int8Features = {};
	if(!(m_extensions & VulkanExtensions::kKHR_shader_float16_int8))
	{
		ANKI_VK_LOGE(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME " is not supported");
		return Error::kFunctionFailed;
	}
	else
	{
		float16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR;
		getPhysicalDevicaFeatures2(float16Int8Features);

		float16Int8Features.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &float16Int8Features;
	}

	// 64bit atomics
	VkPhysicalDeviceShaderAtomicInt64FeaturesKHR atomicInt64Features = {};
	if(!(m_extensions & VulkanExtensions::kKHR_shader_atomic_int64))
	{
		ANKI_VK_LOGW(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME " is not supported or disabled");
		m_capabilities.m_64bitAtomics = false;
	}
	else
	{
		m_capabilities.m_64bitAtomics = true;

		atomicInt64Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR;
		getPhysicalDevicaFeatures2(atomicInt64Features);

		atomicInt64Features.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &atomicInt64Features;
	}

	// VRS
	VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = {};
	if(!(m_extensions & VulkanExtensions::kKHR_fragment_shading_rate))
	{
		ANKI_VK_LOGI(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME " is not supported or disabled");
		m_capabilities.m_vrs = false;
	}
	else
	{
		m_capabilities.m_vrs = true;

		fragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
		getPhysicalDevicaFeatures2(fragmentShadingRateFeatures);

		// Some checks
		if(!fragmentShadingRateFeatures.attachmentFragmentShadingRate || !fragmentShadingRateFeatures.pipelineFragmentShadingRate)
		{
			ANKI_VK_LOGW(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME " doesn't support attachment and/or pipeline rates. Will disable VRS");
			m_capabilities.m_vrs = false;
		}
		else
		{
			// Disable some things
			fragmentShadingRateFeatures.primitiveFragmentShadingRate = false;
		}

		if(m_capabilities.m_vrs)
		{
			VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = {};
			fragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
			getPhysicalDeviceProperties2(fragmentShadingRateProperties);

			if(fragmentShadingRateProperties.minFragmentShadingRateAttachmentTexelSize.width > 16
			   || fragmentShadingRateProperties.minFragmentShadingRateAttachmentTexelSize.height > 16
			   || fragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize.width < 8
			   || fragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize.height < 8)
			{
				ANKI_VK_LOGW(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME
							 " doesn't support 8x8 or 16x16 shading rate attachment texel size. Will disable VRS");
				m_capabilities.m_vrs = false;
			}
			else
			{
				m_capabilities.m_minShadingRateImageTexelSize = max(fragmentShadingRateProperties.minFragmentShadingRateAttachmentTexelSize.width,
																	fragmentShadingRateProperties.minFragmentShadingRateAttachmentTexelSize.height);
			}
		}

		if(m_capabilities.m_vrs)
		{
			fragmentShadingRateFeatures.pNext = const_cast<void*>(ci.pNext);
			ci.pNext = &fragmentShadingRateFeatures;
		}
	}

	// Mesh shaders
	VkPhysicalDeviceMeshShaderFeaturesEXT meshShadersFeatures = {};
	if(!!(m_extensions & VulkanExtensions::kEXT_mesh_shader))
	{
		m_capabilities.m_meshShaders = true;

		meshShadersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
		getPhysicalDevicaFeatures2(meshShadersFeatures);

		if(meshShadersFeatures.taskShader == false)
		{
			ANKI_LOGE(VK_EXT_MESH_SHADER_EXTENSION_NAME " doesn't support task shaders");
			return Error::kFunctionFailed;
		}

		meshShadersFeatures.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &meshShadersFeatures;

		ANKI_VK_LOGI(VK_EXT_MESH_SHADER_EXTENSION_NAME " is supported and enabled");
	}
	else
	{
		ANKI_VK_LOGI(VK_EXT_MESH_SHADER_EXTENSION_NAME " is not supported or disabled ");
	}

	VkPhysicalDeviceMaintenance4FeaturesKHR maintenance4Features = {};
	if(!!(m_extensions & VulkanExtensions::kKHR_maintenance_4))
	{
		maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR;
		maintenance4Features.maintenance4 = true;
		maintenance4Features.pNext = const_cast<void*>(ci.pNext);
		ci.pNext = &maintenance4Features;
	}

	if(!(m_extensions & VulkanExtensions::kKHR_draw_indirect_count) || m_capabilities.m_maxDrawIndirectCount < kMaxU32)
	{
		ANKI_VK_LOGE(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME " not supported or too small maxDrawIndirectCount");
		return Error::kFunctionFailed;
	}

	ANKI_VK_CHECK(vkCreateDevice(m_physicalDevice, &ci, nullptr, &m_device));

	if(!(m_extensions & VulkanExtensions::kKHR_spirv_1_4))
	{
		ANKI_VK_LOGE(VK_KHR_SPIRV_1_4_EXTENSION_NAME " is not supported");
		return Error::kFunctionFailed;
	}

	if(!(m_extensions & VulkanExtensions::kKHR_shader_float_controls))
	{
		ANKI_VK_LOGE(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME " is not supported");
		return Error::kFunctionFailed;
	}

	return Error::kNone;
}

Error GrManagerImpl::initMemory()
{
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

	// Print some info
	ANKI_VK_LOGV("Vulkan memory info:");
	for(U32 i = 0; i < m_memoryProperties.memoryHeapCount; ++i)
	{
		ANKI_VK_LOGV("\tHeap %u size %zu", i, m_memoryProperties.memoryHeaps[i].size);
	}
	for(U32 i = 0; i < m_memoryProperties.memoryTypeCount; ++i)
	{
		ANKI_VK_LOGV("\tMem type %u points to heap %u, flags %" ANKI_PRIb32, i, m_memoryProperties.memoryTypes[i].heapIndex,
					 ANKI_FORMAT_U32(m_memoryProperties.memoryTypes[i].propertyFlags));
	}

	m_gpuMemManager.init(!!(m_extensions & VulkanExtensions::kKHR_buffer_device_address));

	return Error::kNone;
}

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
void* GrManagerImpl::allocateCallback(void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	if(size == 0) [[unlikely]]
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

void* GrManagerImpl::reallocateCallback(void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
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
	ANKI_TRACE_SCOPED_EVENT(VkAcquireImage);

	LockGuard<Mutex> lock(m_globalMtx);

	PerFrame& frame = m_perFrame[m_frame % kMaxFramesInFlight];

	// Create sync objects
	MicroFencePtr fence = m_fenceFactory.newInstance();
	frame.m_acquireSemaphore = m_semaphoreFactory.newInstance(fence, false);

	// Get new image
	uint32_t imageIdx;

	VkResult res = vkAcquireNextImageKHR(m_device, m_crntSwapchain->m_swapchain, UINT64_MAX, frame.m_acquireSemaphore->getHandle(),
										 fence->getHandle(), &imageIdx);

	if(res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ANKI_VK_LOGW("Swapchain is out of date. Will wait for the queue and create a new one");
		for(VkQueue queue : m_queues)
		{
			if(queue)
			{
				vkQueueWaitIdle(queue);
			}
		}
		m_crntSwapchain.reset(nullptr);
		m_crntSwapchain = m_swapchainFactory.newInstance();

		// Can't fail a second time
		ANKI_VK_CHECKF(vkAcquireNextImageKHR(m_device, m_crntSwapchain->m_swapchain, UINT64_MAX, frame.m_acquireSemaphore->getHandle(),
											 fence->getHandle(), &imageIdx));
	}
	else
	{
		ANKI_VK_CHECKF(res);
	}

	m_acquiredImageIdx = U8(imageIdx);
	return m_crntSwapchain->m_textures[imageIdx];
}

void GrManagerImpl::endFrame()
{
	ANKI_TRACE_SCOPED_EVENT(VkPresent);

	LockGuard<Mutex> lock(m_globalMtx);

	PerFrame& frame = m_perFrame[m_frame % kMaxFramesInFlight];

	// Wait for the fence of N-2 frame
	const U waitFrameIdx = (m_frame + 1) % kMaxFramesInFlight;
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
			if(queue)
			{
				vkQueueWaitIdle(queue);
			}
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
	m_gpuMemManager.updateStats();

	// Finalize
	++m_frame;
}

void GrManagerImpl::resetFrame(PerFrame& frame)
{
	frame.m_presentFence.reset(nullptr);
	frame.m_acquireSemaphore.reset(nullptr);
	frame.m_renderSemaphore.reset(nullptr);
}

void GrManagerImpl::flushCommandBuffer(MicroCommandBufferPtr cmdb, Bool cmdbRenderedToSwapchain, WeakArray<MicroSemaphorePtr> userWaitSemaphores,
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
	cmdb->setFence(fence.get());
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &handle;

	// Handle user semaphores
	Array<U64, maxSemaphores> waitTimelineValues;
	Array<U64, maxSemaphores> signalTimelineValues;

	VkTimelineSemaphoreSubmitInfo timelineInfo = {};
	timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timelineInfo.waitSemaphoreValueCount = userWaitSemaphores.getSize();
	timelineInfo.pWaitSemaphoreValues = &waitTimelineValues[0];
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
		userWaitSemaphore->setFence(fence.get());
	}

	if(userSignalSemaphore)
	{
		*userSignalSemaphore = m_semaphoreFactory.newInstance(fence, true);

		signalSemaphores[submit.signalSemaphoreCount++] = (*userSignalSemaphore)->getHandle();
		signalTimelineValues[timelineInfo.signalSemaphoreValueCount++] = (*userSignalSemaphore)->getNextSemaphoreValue();
	}

	// Submit
	{
		// Protect the class, the queue and other stuff
		LockGuard<Mutex> lock(m_globalMtx);

		// Do some special stuff for the last command buffer
		PerFrame& frame = m_perFrame[m_frame % kMaxFramesInFlight];
		if(cmdbRenderedToSwapchain)
		{
			// Wait semaphore
			waitSemaphores[submit.waitSemaphoreCount] = frame.m_acquireSemaphore->getHandle();

			// That depends on how we use the swapchain img. Be a bit conservative
			waitStages[submit.waitSemaphoreCount] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			++submit.waitSemaphoreCount;

			// Refresh the fence because the semaphore can't be recycled until the current submission is done
			frame.m_acquireSemaphore->setFence(fence.get());

			// Create the semaphore to signal
			ANKI_ASSERT(!frame.m_renderSemaphore && "Only one begin/end render pass is allowed with the default fb");
			frame.m_renderSemaphore = m_semaphoreFactory.newInstance(fence, false);

			signalSemaphores[submit.signalSemaphoreCount++] = frame.m_renderSemaphore->getHandle();

			// Increment the timeline values as well because the spec wants a dummy value even for non-timeline semaphores
			signalTimelineValues[timelineInfo.signalSemaphoreValueCount++] = 0;

			// Update the frame fence
			frame.m_presentFence = fence;

			// Update the swapchain's fence
			m_crntSwapchain->setFence(fence.get());

			frame.m_queueWroteToSwapchainImage = cmdb->getVulkanQueueType();
		}

		// Submit
		ANKI_TRACE_SCOPED_EVENT(VkQueueSubmit);
		ANKI_VK_CHECKF(vkQueueSubmit(m_queues[cmdb->getVulkanQueueType()], 1, &submit, fence->getHandle()));

		if(wait)
		{
			vkQueueWaitIdle(m_queues[cmdb->getVulkanQueueType()]);
		}
	}

	// Garbage work
	if(cmdbRenderedToSwapchain)
	{
		m_frameGarbageCollector.setNewFrame(fence);
	}
}

void GrManagerImpl::finish()
{
	LockGuard<Mutex> lock(m_globalMtx);
	for(VkQueue queue : m_queues)
	{
		if(queue)
		{
			vkQueueWaitIdle(queue);
		}
	}
}

void GrManagerImpl::trySetVulkanHandleName(CString name, VkObjectType type, U64 handle) const
{
	if(name && name.getLength() && !!(m_extensions & VulkanExtensions::kEXT_debug_utils))
	{
		VkDebugUtilsObjectNameInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		info.objectHandle = handle;
		info.objectType = type;
		info.pObjectName = name.cstr();

		vkSetDebugUtilsObjectNameEXT(m_device, &info);
	}
}

VkBool32 GrManagerImpl::debugReportCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
											   [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
											   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData)
{
#if ANKI_PLATFORM_MOBILE
	if(pCallbackData->messageIdNumber == 101294395)
	{
		// Interface mismatch error. Eg vert shader is writing to varying that is not consumed by frag. Ignore this
		// stupid error because I'm not going to create more shader variants to fix it. Especially when mobile drivers
		// do linking anyway. On desktop just enable the maintenance4 extension
		return false;
	}
#endif

	// Get all names of affected objects
	GrString objectNames;
	if(pCallbackData->objectCount)
	{
		for(U32 i = 0; i < pCallbackData->objectCount; ++i)
		{
			const Char* name = pCallbackData->pObjects[i].pObjectName;
			objectNames += (name) ? name : "?";
			if(i < pCallbackData->objectCount - 1)
			{
				objectNames += ", ";
			}
		}
	}
	else
	{
		objectNames = "N/A";
	}

	if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		ANKI_VK_LOGE("VK debug report: %s. Affected objects: %s", pCallbackData->pMessage, objectNames.cstr());
	}
	else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		ANKI_VK_LOGW("VK debug report: %s. Affected objects: %s", pCallbackData->pMessage, objectNames.cstr());
	}
	else
	{
		ANKI_VK_LOGI("VK debug report: %s. Affected objects: %s", pCallbackData->pMessage, objectNames.cstr());
	}

	return false;
}

void GrManagerImpl::printPipelineShaderInfo(VkPipeline ppline, CString name, U64 hash) const
{
	if(printPipelineShaderInfoInternal(ppline, name, hash))
	{
		ANKI_VK_LOGE("Ignoring previous errors");
	}
}

Error GrManagerImpl::printPipelineShaderInfoInternal(VkPipeline ppline, CString name, U64 hash) const
{
	if(!!(m_extensions & VulkanExtensions::kKHR_pipeline_executable_properties))
	{
		GrStringList log;

		VkPipelineInfoKHR pplineInf = {};
		pplineInf.sType = VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR;
		pplineInf.pipeline = ppline;
		U32 executableCount = 0;
		ANKI_VK_CHECK(vkGetPipelineExecutablePropertiesKHR(m_device, &pplineInf, &executableCount, nullptr));
		GrDynamicArray<VkPipelineExecutablePropertiesKHR> executableProps;
		executableProps.resize(executableCount);
		for(VkPipelineExecutablePropertiesKHR& prop : executableProps)
		{
			prop = {};
			prop.sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR;
		}
		ANKI_VK_CHECK(vkGetPipelineExecutablePropertiesKHR(m_device, &pplineInf, &executableCount, &executableProps[0]));

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
			GrDynamicArray<VkPipelineExecutableStatisticKHR> stats;
			stats.resize(statCount);
			for(VkPipelineExecutableStatisticKHR& s : stats)
			{
				s = {};
				s.sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR;
			}
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

		GrString finalLog;
		log.join("", finalLog);
		ANKI_VK_LOGV("%s", finalLog.cstr());
	}

	return Error::kNone;
}

} // end namespace anki
