// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Vulkan/Common.h>
#include <AnKi/Gr/Vulkan/GpuMemoryManager.h>
#include <AnKi/Gr/Vulkan/SemaphoreFactory.h>
#include <AnKi/Gr/Vulkan/DeferredBarrierFactory.h>
#include <AnKi/Gr/Vulkan/FenceFactory.h>
#include <AnKi/Gr/Vulkan/SamplerFactory.h>
#include <AnKi/Gr/Vulkan/QueryFactory.h>
#include <AnKi/Gr/Vulkan/DescriptorSet.h>
#include <AnKi/Gr/Vulkan/CommandBufferFactory.h>
#include <AnKi/Gr/Vulkan/SwapchainFactory.h>
#include <AnKi/Gr/Vulkan/PipelineLayout.h>
#include <AnKi/Gr/Vulkan/PipelineCache.h>
#include <AnKi/Gr/Vulkan/DescriptorSet.h>
#include <AnKi/Gr/Vulkan/FrameGarbageCollector.h>
#include <AnKi/Util/HashMap.h>
#include <AnKi/Util/File.h>

namespace anki {

/// @note Disable that because it crashes Intel drivers
#define ANKI_GR_MANAGER_DEBUG_MEMMORY ANKI_EXTRA_CHECKS && 0

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

	Error init(const GrManagerInitInfo& cfg);

	ConstWeakArray<U32> getQueueFamilies() const
	{
		const Bool hasAsyncCompute = m_queueFamilyIndices[VulkanQueueType::kCompute] != kMaxU32;
		return (hasAsyncCompute) ? m_queueFamilyIndices : ConstWeakArray<U32>(&m_queueFamilyIndices[0], 1);
	}

	const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const
	{
		return m_devProps.properties;
	}

	const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& getPhysicalDeviceRayTracingProperties() const
	{
		return m_rtPipelineProps;
	}

	TexturePtr acquireNextPresentableTexture();

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

	VkInstance getInstance() const
	{
		ANKI_ASSERT(m_instance);
		return m_instance;
	}

	/// @name object_creation
	/// @{

	CommandBufferFactory& getCommandBufferFactory()
	{
		return m_cmdbFactory;
	}

	const CommandBufferFactory& getCommandBufferFactory() const
	{
		return m_cmdbFactory;
	}

	SamplerFactory& getSamplerFactory()
	{
		return m_samplerFactory;
	}
	/// @}

	void flushCommandBuffer(MicroCommandBufferPtr cmdb, Bool cmdbRenderedToSwapchain,
							WeakArray<MicroSemaphorePtr> waitSemaphores, MicroSemaphorePtr* signalSemaphore,
							Bool wait = false);

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

	QueryFactory& getOcclusionQueryFactory()
	{
		return m_occlusionQueryFactory;
	}

	QueryFactory& getTimestampQueryFactory()
	{
		return m_timestampQueryFactory;
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

#if ANKI_WINDOWING_SYSTEM_HEADLESS
	void getNativeWindowSize(U32& width, U32& height) const
	{
		width = m_nativeWindowWidth;
		height = m_nativeWindowHeight;
		ANKI_ASSERT(width && height);
	}
#endif

	/// @name Debug report
	/// @{
	void beginMarker(VkCommandBuffer cmdb, CString name, Vec3 color = Vec3(1.0f, 0.0f, 0.0f)) const
	{
		ANKI_ASSERT(cmdb);
		if(!!(m_extensions & VulkanExtensions::kEXT_debug_utils))
		{
			VkDebugUtilsLabelEXT label = {};
			label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			label.pLabelName = name.cstr();
			label.color[0] = color[0];
			label.color[1] = color[1];
			label.color[2] = color[2];
			label.color[3] = 1.0f;
			vkCmdBeginDebugUtilsLabelEXT(cmdb, &label);
		}
	}

	void endMarker(VkCommandBuffer cmdb) const
	{
		ANKI_ASSERT(cmdb);
		if(!!(m_extensions & VulkanExtensions::kEXT_debug_utils))
		{
			vkCmdEndDebugUtilsLabelEXT(cmdb);
		}
	}

	void trySetVulkanHandleName(CString name, VkObjectType type, U64 handle) const;

	void trySetVulkanHandleName(CString name, VkObjectType type, void* handle) const
	{
		trySetVulkanHandleName(name, type, U64(ptrToNumber(handle)));
	}
	/// @}

	/// @note It's thread-safe.
	void printPipelineShaderInfo(VkPipeline ppline, CString name, ShaderTypeBit stages, U64 hash = 0) const;

	FrameGarbageCollector& getFrameGarbageCollector()
	{
		return m_frameGarbageCollector;
	}

#if ANKI_PLATFORM_MOBILE
	Mutex* getGlobalCreatePipelineMutex() const
	{
		return m_globalCreatePipelineMtx;
	}
#endif

private:
	U64 m_frame = 0;

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
	VkAllocationCallbacks m_debugAllocCbs;
	static constexpr U32 MAX_ALLOC_ALIGNMENT = 64;
	static constexpr PtrSize ALLOC_SIG = 0xF00B00;

	struct alignas(MAX_ALLOC_ALIGNMENT) AllocHeader
	{
		PtrSize m_sig;
		PtrSize m_size;
	};
#endif

	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VulkanExtensions m_extensions = VulkanExtensions::kNone;
	VkDevice m_device = VK_NULL_HANDLE;
	VulkanQueueFamilies m_queueFamilyIndices = {kMaxU32, kMaxU32};
	Array<VkQueue, U32(VulkanQueueType::kCount)> m_queues = {};
	Mutex m_globalMtx;

	VkPhysicalDeviceProperties2 m_devProps = {};
	VkPhysicalDeviceAccelerationStructurePropertiesKHR m_accelerationStructureProps = {};
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rtPipelineProps = {};
	VkPhysicalDeviceFeatures m_devFeatures = {};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructureFeatures = {};
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_rtPipelineFeatures = {};
	VkPhysicalDeviceRayQueryFeaturesKHR m_rayQueryFeatures = {};
	VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR m_pplineExecutablePropertiesFeatures = {};
	VkPhysicalDeviceDescriptorIndexingFeatures m_descriptorIndexingFeatures = {};
	VkPhysicalDeviceBufferDeviceAddressFeaturesKHR m_deviceBufferFeatures = {};
	VkPhysicalDeviceScalarBlockLayoutFeaturesEXT m_scalarBlockLayout = {};
	VkPhysicalDeviceTimelineSemaphoreFeaturesKHR m_timelineSemaphoreFeatures = {};
	VkPhysicalDeviceShaderFloat16Int8FeaturesKHR m_float16Int8Features = {};
	VkPhysicalDeviceShaderAtomicInt64FeaturesKHR m_atomicInt64Features = {};
	VkPhysicalDeviceFragmentShadingRateFeaturesKHR m_fragmentShadingRateFeatures = {};

	VkDebugUtilsMessengerEXT m_debugUtilsMessager = VK_NULL_HANDLE;

	PFN_vkGetShaderInfoAMD m_pfnGetShaderInfoAMD = nullptr;
	mutable File m_shaderStatsFile;
	mutable SpinLock m_shaderStatsFileMtx;

	/// @name Surface_related
	/// @{
	class PerFrame
	{
	public:
		MicroFencePtr m_presentFence;
		MicroSemaphorePtr m_acquireSemaphore;

		/// Signaled by the submit that renders to the default FB. Present waits for it.
		MicroSemaphorePtr m_renderSemaphore;

		VulkanQueueType m_queueWroteToSwapchainImage = VulkanQueueType::kCount;
	};

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
#if ANKI_WINDOWING_SYSTEM_HEADLESS
	U32 m_nativeWindowWidth = 0;
	U32 m_nativeWindowHeight = 0;
#endif
	MicroSwapchainPtr m_crntSwapchain;
	U8 m_acquiredImageIdx = kMaxU8;

	Array<PerFrame, kMaxFramesInFlight> m_perFrame;
	/// @}

	/// @name Memory
	/// @{
	VkPhysicalDeviceMemoryProperties m_memoryProperties;

	/// The main allocator.
	GpuMemoryManager m_gpuMemManager;
	/// @}

	CommandBufferFactory m_cmdbFactory;

	FenceFactory m_fenceFactory;
	SemaphoreFactory m_semaphoreFactory;
	DeferredBarrierFactory m_barrierFactory;
	SamplerFactory m_samplerFactory;
	/// @}

	SwapchainFactory m_swapchainFactory;

	PipelineLayoutFactory m_pplineLayoutFactory;

	DescriptorSetFactory m_descrFactory;

	QueryFactory m_occlusionQueryFactory;
	QueryFactory m_timestampQueryFactory;

	PipelineCache m_pplineCache;

	FrameGarbageCollector m_frameGarbageCollector;

#if ANKI_PLATFORM_MOBILE
	Mutex* m_globalCreatePipelineMtx = nullptr;
#endif

	Error initInternal(const GrManagerInitInfo& init);
	Error initInstance();
	Error initSurface(const GrManagerInitInfo& init);
	Error initDevice(const GrManagerInitInfo& init);
	Error initMemory();

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
	static void* allocateCallback(void* userData, size_t size, size_t alignment,
								  VkSystemAllocationScope allocationScope);

	static void* reallocateCallback(void* userData, void* original, size_t size, size_t alignment,
									VkSystemAllocationScope allocationScope);

	static void freeCallback(void* userData, void* ptr);
#endif

	void resetFrame(PerFrame& frame);

	static VkBool32 debugReportCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
										   VkDebugUtilsMessageTypeFlagsEXT messageTypes,
										   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	Error printPipelineShaderInfoInternal(VkPipeline ppline, CString name, ShaderTypeBit stages, U64 hash) const;
};
/// @}

} // end namespace anki
