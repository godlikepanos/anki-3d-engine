// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/Common.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>
#include <anki/gr/vulkan/SemaphoreFactory.h>
#include <anki/gr/vulkan/DeferredBarrierFactory.h>
#include <anki/gr/vulkan/FenceFactory.h>
#include <anki/gr/vulkan/SamplerFactory.h>
#include <anki/gr/vulkan/QueryExtra.h>
#include <anki/gr/vulkan/DescriptorSet.h>
#include <anki/gr/vulkan/CommandBufferFactory.h>
#include <anki/gr/vulkan/SwapchainFactory.h>
#include <anki/gr/vulkan/PipelineLayout.h>
#include <anki/gr/vulkan/PipelineCache.h>
#include <anki/util/HashMap.h>

namespace anki
{

#define ANKI_GR_MANAGER_DEBUG_MEMMORY ANKI_EXTRA_CHECKS

// Forward
class TextureFallbackUploader;
class ConfigSet;

/// @addtogroup vulkan
/// @{

/// Vulkan implementation of GrManager.
class GrManagerImpl : public GrManager
{
public:
	GrManagerImpl()
	{
	}

	~GrManagerImpl();

	ANKI_USE_RESULT Error init(const GrManagerInitInfo& cfg);

	U32 getGraphicsQueueFamily() const
	{
		return m_queueIdx;
	}

	const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const
	{
		return m_devProps;
	}

	void beginFrame();

	void endFrame();

	void finish();

	VkDevice getDevice() const
	{
		ANKI_ASSERT(m_device);
		return m_device;
	}

	VkPhysicalDevice getPhysicalDevice() const
	{
		ANKI_ASSERT(m_physicalDevice);
		return m_physicalDevice;
	}

	/// @name object_creation
	/// @{

	CommandBufferFactory& getCommandBufferFactory()
	{
		return m_cmdbFactory;
	}

	MicroFencePtr newFence()
	{
		return m_fences.newInstance();
	}

	SamplerFactory& getSamplerFactory()
	{
		return m_samplerFactory;
	}
	/// @}

	void flushCommandBuffer(CommandBufferPtr ptr, FencePtr* fence, Bool wait = false);

	/// @name Memory
	/// @{
	GpuMemoryManager& getGpuMemoryManager()
	{
		return m_gpuMemManager;
	}

	const GpuMemoryManager& getGpuMemoryManager() const
	{
		return m_gpuMemManager;
	}

	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const
	{
		return m_memoryProperties;
	}
	/// @}

	QueryAllocator& getQueryAllocator()
	{
		return m_queryAlloc;
	}

	Bool getR8g8b8ImagesSupported() const
	{
		return m_r8g8b8ImagesSupported;
	}

	Bool getS8ImagesSupported() const
	{
		return m_s8ImagesSupported;
	}

	Bool getD24S8ImagesSupported() const
	{
		return m_d24S8ImagesSupported;
	}

	DescriptorSetFactory& getDescriptorSetFactory()
	{
		return m_descrFactory;
	}

	VkPipelineCache getPipelineCache() const
	{
		ANKI_ASSERT(m_pplineCache.m_cacheHandle);
		return m_pplineCache.m_cacheHandle;
	}

	PipelineLayoutFactory& getPipelineLayoutFactory()
	{
		return m_pplineLayoutFactory;
	}

	VulkanExtensions getExtensions() const
	{
		return m_extensions;
	}

	MicroSwapchainPtr getSwapchain() const
	{
		return m_crntSwapchain;
	}

	VkSurfaceKHR getSurface() const
	{
		ANKI_ASSERT(m_surface);
		return m_surface;
	}

	U32 getGraphicsQueueIndex() const
	{
		return m_queueIdx;
	}

	void trySetVulkanHandleName(CString name, VkDebugReportObjectTypeEXT type, U64 handle) const;

	void trySetVulkanHandleName(CString name, VkDebugReportObjectTypeEXT type, void* handle) const
	{
		trySetVulkanHandleName(name, type, U64(ptrToNumber(handle)));
	}

	StringAuto tryGetVulkanHandleName(U64 handle) const;

	StringAuto tryGetVulkanHandleName(void* handle) const
	{
		return tryGetVulkanHandleName(U64(ptrToNumber(handle)));
	}

private:
	U64 m_frame = 0;

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
	VkAllocationCallbacks m_debugAllocCbs;
	static const U32 MAX_ALLOC_ALIGNMENT = 64;
	static const PtrSize ALLOC_SIG = 0xF00B00;

	struct alignas(MAX_ALLOC_ALIGNMENT) AllocHeader
	{
		PtrSize m_sig;
		PtrSize m_size;
	};
#endif

	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT m_debugCallback = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VulkanExtensions m_extensions = VulkanExtensions::NONE;
	VkDevice m_device = VK_NULL_HANDLE;
	U32 m_queueIdx = MAX_U32;
	VkQueue m_queue = VK_NULL_HANDLE;
	Mutex m_globalMtx;

	VkPhysicalDeviceProperties m_devProps = {};
	VkPhysicalDeviceFeatures m_devFeatures = {};

	PFN_vkDebugMarkerSetObjectNameEXT m_pfnDebugMarkerSetObjectNameEXT = nullptr;

	/// @name Surface_related
	/// @{
	class PerFrame
	{
	public:
		MicroFencePtr m_presentFence;
		MicroSemaphorePtr m_acquireSemaphore;

		/// The semaphore that the submit that renders to the default FB.
		MicroSemaphorePtr m_renderSemaphore;
	};

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	MicroSwapchainPtr m_crntSwapchain;

	Array<PerFrame, MAX_FRAMES_IN_FLIGHT> m_perFrame;
	/// @}

	/// @name Memory
	/// @{
	VkPhysicalDeviceMemoryProperties m_memoryProperties;

	/// The main allocator.
	GpuMemoryManager m_gpuMemManager;
	/// @}

	CommandBufferFactory m_cmdbFactory;

	FenceFactory m_fences;
	SemaphoreFactory m_semaphores;
	DeferredBarrierFactory m_barrierFactory;
	SamplerFactory m_samplerFactory;
	/// @}

	SwapchainFactory m_swapchainFactory;

	PipelineLayoutFactory m_pplineLayoutFactory;

	DescriptorSetFactory m_descrFactory;

	QueryAllocator m_queryAlloc;

	PipelineCache m_pplineCache;

	Bool8 m_r8g8b8ImagesSupported = false;
	Bool8 m_s8ImagesSupported = false;
	Bool8 m_d24S8ImagesSupported = false;

	mutable HashMap<U64, StringAuto> m_vkHandleToName;
	mutable SpinLock m_vkHandleToNameLock;

	ANKI_USE_RESULT Error initInternal(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initInstance(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initSurface(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initDevice(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initFramebuffers(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initMemory(const ConfigSet& cfg);

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
	static void* allocateCallback(
		void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);

	static void* reallocateCallback(
		void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);

	static void freeCallback(void* userData, void* ptr);
#endif

	void resetFrame(PerFrame& frame);

	static VkBool32 debugReportCallbackEXT(VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		uint64_t object,
		size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData);
};
/// @}

} // end namespace anki
