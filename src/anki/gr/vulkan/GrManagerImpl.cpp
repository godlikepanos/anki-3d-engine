// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/GrManager.h>

#include <anki/gr/Pipeline.h>
#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/CommandBuffer.h>

#include <anki/core/Config.h>
#include <glslang/Public/ShaderLang.h>

namespace anki
{

#define ANKI_GR_MANAGER_DEBUG_MEMMORY ANKI_DEBUG

GrManagerImpl::~GrManagerImpl()
{
	// FIRST THING: wait for the GPU
	if(m_queue)
	{
		LockGuard<Mutex> lock(m_queueSubmitMtx);
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

	// THIRD THING: Continue with the rest
	m_rpCreator.destroy();

	if(m_globalPipelineLayout)
	{
		vkDestroyPipelineLayout(m_device, m_globalPipelineLayout, nullptr);
	}

	m_dsetAlloc.destroy();
	m_transientMem.destroy();
	m_gpuAlloc.destroy();

	m_semaphores.destroy(); // Destroy before fences
	m_fences.destroy();

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
		vkDestroyInstance(m_instance, nullptr);
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
		ANKI_LOGE("Vulkan initialization failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	return ErrorCode::NONE;
}

Error GrManagerImpl::initInternal(const GrManagerInitInfo& init)
{
	ANKI_LOGI("Initializing Vulkan backend");
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
	ANKI_CHECK(m_dsetAlloc.init(m_device));
	ANKI_CHECK(initGlobalPplineLayout());

	for(PerFrame& f : m_perFrame)
	{
		resetFrame(f);
	}

	glslang::InitializeProcess();
	m_fences.init(getAllocator(), m_device);
	m_semaphores.init(getAllocator(), m_device);

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
			ANKI_LOGI("R8G8B8 Images are not supported. Will workaround this");
			m_r8g8b8ImagesSupported = false;
		}
		else
		{
			ANKI_ASSERT(res == VK_SUCCESS);
			ANKI_LOGI("R8G8B8 Images are supported");
			m_r8g8b8ImagesSupported = true;
		}
	}

	return ErrorCode::NONE;
}

Error GrManagerImpl::initInstance(const GrManagerInitInfo& init)
{
	// Create the instance
	//
	static Array<const char*, 8> LAYERS = {{"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_LUNARG_image",
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_GOOGLE_unique_objects",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_standard_validation"}};

	static Array<const char*, 2> EXTENSIONS = {{VK_KHR_SURFACE_EXTENSION_NAME,
#if ANKI_OS == ANKI_OS_LINUX
		VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif ANKI_OS == ANKI_OS_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#else
#error TODO
#endif
	}};

	VkApplicationInfo app = {};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "unamed";
	app.applicationVersion = 1;
	app.pEngineName = "AnKi 3D Engine";
	app.engineVersion = (ANKI_VERSION_MAJOR << 1) | ANKI_VERSION_MINOR;
	app.apiVersion = VK_MAKE_VERSION(1, 0, 3);

	VkInstanceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pApplicationInfo = &app;

	if(init.m_config->getNumber("debugContext"))
	{
		ANKI_LOGI("VK: Will enable debug layers");
		ci.enabledLayerCount = LAYERS.getSize();
		ci.ppEnabledLayerNames = &LAYERS[0];
	}

	ci.enabledExtensionCount = EXTENSIONS.getSize();
	ci.ppEnabledExtensionNames = &EXTENSIONS[0];

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
	VkAllocationCallbacks allocCbs = {};
	VkAllocationCallbacks* pallocCbs = &allocCbs;
	allocCbs.pUserData = this;
	allocCbs.pfnAllocation = allocateCallback;
	allocCbs.pfnReallocation = reallocateCallback;
	allocCbs.pfnFree = freeCallback;
#else
	VkAllocationCallbacks* pallocCbs = nullptr;
#endif

	ANKI_VK_CHECK(vkCreateInstance(&ci, pallocCbs, &m_instance));

	// Create the physical device
	//
	uint32_t count = 0;
	ANKI_VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &count, nullptr));
	ANKI_LOGI("VK: Number of physical devices: %u", count);
	if(count < 1)
	{
		ANKI_LOGE("Wrong number of physical devices");
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
	}
	ANKI_LOGI("GPU vendor is %s", &GPU_VENDOR_STR[m_vendor][0]);

	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_devFeatures);

	return ErrorCode::NONE;
}

Error GrManagerImpl::initDevice(const GrManagerInitInfo& init)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, nullptr);
	ANKI_LOGI("VK: Number of queue families: %u\n", count);

	DynamicArrayAuto<VkQueueFamilyProperties> queueInfos(getAllocator());
	queueInfos.create(count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, &queueInfos[0]);

	uint32_t desiredFamilyIdx = MAX_U32;
	const VkQueueFlags DESITED_QUEUE_FLAGS = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
	for(U i = 0; i < count; ++i)
	{
		if((queueInfos[i].queueFlags & (DESITED_QUEUE_FLAGS)) == DESITED_QUEUE_FLAGS)
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
		ANKI_LOGE("Couldn't find a queue family with graphics+compute+present."
				  "The assumption was wrong. The code needs rework");
		return ErrorCode::FUNCTION_FAILED;
	}

	m_queueIdx = desiredFamilyIdx;

	F32 priority = 1.0;
	VkDeviceQueueCreateInfo q = {};
	q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	q.queueFamilyIndex = desiredFamilyIdx;
	q.queueCount = 1;
	q.pQueuePriorities = &priority;

	static Array<const char*, 1> DEV_EXTENSIONS = {{VK_KHR_SWAPCHAIN_EXTENSION_NAME}};

	VkDeviceCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.queueCreateInfoCount = 1;
	ci.pQueueCreateInfos = &q;
	ci.enabledExtensionCount = DEV_EXTENSIONS.getSize();
	ci.ppEnabledExtensionNames = &DEV_EXTENSIONS[0];
	ci.pEnabledFeatures = &m_devFeatures;

	ANKI_VK_CHECK(vkCreateDevice(m_physicalDevice, &ci, nullptr, &m_device));

	return ErrorCode::NONE;
}

Error GrManagerImpl::initSwapchain(const GrManagerInitInfo& init)
{
	VkSurfaceCapabilitiesKHR surfaceProperties;
	ANKI_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceProperties));

	if(surfaceProperties.currentExtent.width == MAX_U32 || surfaceProperties.currentExtent.height == MAX_U32)
	{
		ANKI_LOGE("Wrong surface size");
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
		ANKI_LOGE("Surface format not found");
		return ErrorCode::FUNCTION_FAILED;
	}

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
	ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	ci.clipped = false;
	ci.oldSwapchain = VK_NULL_HANDLE;

	ANKI_VK_CHECK(vkCreateSwapchainKHR(m_device, &ci, nullptr, &m_swapchain));

	// Get images
	uint32_t count = 0;
	ANKI_VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &count, nullptr));
	if(count != MAX_FRAMES_IN_FLIGHT)
	{
		ANKI_LOGE("Requested a swapchain with %u images but got one with %u", MAX_FRAMES_IN_FLIGHT, count);
		return ErrorCode::FUNCTION_FAILED;
	}

	ANKI_LOGI("VK: Swapchain images count %u", count);

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

Error GrManagerImpl::initGlobalPplineLayout()
{
	Array<VkDescriptorSetLayout, MAX_BOUND_RESOURCE_GROUPS> sets = {
		{m_dsetAlloc.getGlobalDescriptorSetLayout(), m_dsetAlloc.getGlobalDescriptorSetLayout()}};

	VkPipelineLayoutCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.setLayoutCount = MAX_BOUND_RESOURCE_GROUPS;
	ci.pSetLayouts = &sets[0];
	ci.pushConstantRangeCount = 0;
	ci.pPushConstantRanges = nullptr;

	ANKI_VK_CHECK(vkCreatePipelineLayout(m_device, &ci, nullptr, &m_globalPipelineLayout));

	return ErrorCode::NONE;
}

Error GrManagerImpl::initMemory(const ConfigSet& cfg)
{
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

	m_gpuAlloc.init(m_physicalDevice, m_device, getAllocator());

	// Transient mem
	ANKI_CHECK(m_transientMem.init(cfg));

	return ErrorCode::NONE;
}

void* GrManagerImpl::allocateCallback(
	void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	ANKI_ASSERT(userData);
	GrManagerImpl* self = static_cast<GrManagerImpl*>(userData);
	return self->getAllocator().getMemoryPool().allocate(size, alignment);
}

void* GrManagerImpl::reallocateCallback(
	void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	ANKI_ASSERT(0 && "TODO");
	return nullptr;
}

void GrManagerImpl::freeCallback(void* userData, void* ptr)
{
	if(ptr)
	{
		ANKI_ASSERT(userData);
		GrManagerImpl* self = static_cast<GrManagerImpl*>(userData);
		self->getAllocator().getMemoryPool().free(ptr);
	}
}

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
		ANKI_LOGW("Nobody draw to the default framebuffer");
	}

	// Present
	VkResult res;
	uint32_t imageIdx = m_frame % MAX_FRAMES_IN_FLIGHT;
	VkPresentInfoKHR present = {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.waitSemaphoreCount = (frame.m_renderSemaphore) ? 1 : 0;
	present.pWaitSemaphores = (frame.m_renderSemaphore) ? &frame.m_renderSemaphore->getHandle() : nullptr;
	present.swapchainCount = 1;
	present.pSwapchains = &m_swapchain;
	present.pImageIndices = &imageIdx;
	present.pResults = &res;

	{
		LockGuard<Mutex> lock(m_queueSubmitMtx);
		ANKI_VK_CHECKF(vkQueuePresentKHR(m_queue, &present));
	}
	ANKI_VK_CHECKF(res);

	m_transientMem.endFrame();

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
			ANKI_LOGF("Cannot recover");
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

void GrManagerImpl::flushCommandBuffer(CommandBufferPtr cmdb,
	SemaphorePtr signalSemaphore,
	WeakArray<SemaphorePtr> waitSemaphores,
	WeakArray<VkPipelineStageFlags> waitPplineStages)
{
	CommandBufferImpl& impl = cmdb->getImplementation();
	VkCommandBuffer handle = impl.getHandle();

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	FencePtr fence = newFence();
	PerFrame& frame = m_perFrame[m_frame % MAX_FRAMES_IN_FLIGHT];

	if(signalSemaphore)
	{
		ANKI_ASSERT(0 && "TODO");
		/*submit.pSignalSemaphores = &signalSemaphore->getHandle();
		submit.signalSemaphoreCount = 1;
		signalSemaphore->setFence(fence);*/
	}

	Array<VkSemaphore, 16> allWaitSemaphores;
	Array<VkPipelineStageFlags, 16> allWaitPplineStages;
	for(U i = 0; i < waitSemaphores.getSize(); ++i)
	{
		ANKI_ASSERT(0 && "TODO");
		/*ANKI_ASSERT(waitSemaphores[i]);
		allWaitSemaphores[submit.waitSemaphoreCount] = waitSemaphores[i]->getHandle();
		allWaitPplineStages[submit.waitSemaphoreCount] = waitPplineStages[i];
		++submit.waitSemaphoreCount;*/
	}

	// Do some special stuff for the last command buffer
	if(impl.renderedToDefaultFramebuffer())
	{
		allWaitSemaphores[submit.waitSemaphoreCount] = frame.m_acquireSemaphore->getHandle();
		allWaitPplineStages[submit.waitSemaphoreCount] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		++submit.waitSemaphoreCount;

		// Create the semaphore to signal
		ANKI_ASSERT(!frame.m_renderSemaphore && "Only one begin/end render pass is allowed with the default fb");
		frame.m_renderSemaphore = m_semaphores.newInstance(fence);

		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &frame.m_renderSemaphore->getHandle();

		frame.m_presentFence = fence;
	}

	submit.pWaitSemaphores = &allWaitSemaphores[0];
	submit.pWaitDstStageMask = &allWaitPplineStages[0];

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &handle;

	// Lock to submit
	LockGuard<Mutex> lock(m_queueSubmitMtx);

	frame.m_cmdbsSubmitted.pushBack(getAllocator(), cmdb);

	ANKI_TRACE_START_EVENT(VK_QUEUE_SUBMIT);
	ANKI_VK_CHECKF(vkQueueSubmit(m_queue, 1, &submit, fence->getHandle()));
	ANKI_TRACE_STOP_EVENT(VK_QUEUE_SUBMIT);
}

} // end namespace anki
