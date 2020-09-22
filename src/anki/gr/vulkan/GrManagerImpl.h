// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/gr/vulkan/QueryFactory.h>
#include <anki/gr/vulkan/DescriptorSet.h>
#include <anki/gr/vulkan/CommandBufferFactory.h>
#include <anki/gr/vulkan/SwapchainFactory.h>
#include <anki/gr/vulkan/PipelineLayout.h>
#include <anki/gr/vulkan/PipelineCache.h>
#include <anki/gr/vulkan/DescriptorSet.h>
#include <anki/util/HashMap.h>
#include <anki/util/File.h>

namespace anki
{

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

	ANKI_USE_RESULT Error init(const GrManagerInitInfo& cfg);

	U32 getGraphicsQueueFamily() const
	{
		return m_queueIdx;
	}

	const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const
	{
		return m_devProps;
	}

	const VkPhysicalDeviceRayTracingPropertiesKHR& getPhysicalDeviceRayTracingProperties() const
	{
		return m_rtProps;
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

	QueryFactory& getOcclusionQueryFactory()
	{
		return m_occlusionQueryFactory;
	}

	QueryFactory& getTimestampQueryFactory()
	{
		return m_timestampQueryFactory;
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

	/// @name Debug report
	/// @{
	void beginMarker(VkCommandBuffer cmdb, CString name) const
	{
		ANKI_ASSERT(cmdb);
		if(m_pfnCmdDebugMarkerBeginEXT)
		{
			VkDebugMarkerMarkerInfoEXT markerInfo = {};
			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			markerInfo.color[0] = 1.0f;
			markerInfo.pMarkerName = (!name.isEmpty() && name.getLength() > 0) ? name.cstr() : "Unnamed";
			m_pfnCmdDebugMarkerBeginEXT(cmdb, &markerInfo);
		}
	}

	void endMarker(VkCommandBuffer cmdb) const
	{
		ANKI_ASSERT(cmdb);
		if(m_pfnCmdDebugMarkerEndEXT)
		{
			m_pfnCmdDebugMarkerEndEXT(cmdb);
		}
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
	/// @}

	void printPipelineShaderInfo(VkPipeline ppline, CString name, ShaderTypeBit stages, U64 hash = 0) const;

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
	VkPhysicalDeviceRayTracingPropertiesKHR m_rtProps = {};
	VkPhysicalDeviceFeatures m_devFeatures = {};
	VkPhysicalDeviceRayTracingFeaturesKHR m_rtFeatures = {};
	VkPhysicalDeviceVulkan12Features m_12Features = {};

	PFN_vkDebugMarkerSetObjectNameEXT m_pfnDebugMarkerSetObjectNameEXT = nullptr;
	PFN_vkCmdDebugMarkerBeginEXT m_pfnCmdDebugMarkerBeginEXT = nullptr;
	PFN_vkCmdDebugMarkerEndEXT m_pfnCmdDebugMarkerEndEXT = nullptr;
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

		/// The semaphore that the submit that renders to the default FB.
		MicroSemaphorePtr m_renderSemaphore;
	};

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	MicroSwapchainPtr m_crntSwapchain;
	U8 m_acquiredImageIdx = MAX_U8;

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

	QueryFactory m_occlusionQueryFactory;
	QueryFactory m_timestampQueryFactory;

	PipelineCache m_pplineCache;

	Bool m_r8g8b8ImagesSupported = false;
	Bool m_s8ImagesSupported = false;
	Bool m_d24S8ImagesSupported = false;

	mutable HashMap<U64, StringAuto> m_vkHandleToName;
	mutable SpinLock m_vkHandleToNameLock;

	ANKI_USE_RESULT Error initInternal(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initInstance(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initSurface(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initDevice(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initFramebuffers(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initMemory(const ConfigSet& cfg);

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
	static void* allocateCallback(void* userData, size_t size, size_t alignment,
								  VkSystemAllocationScope allocationScope);

	static void* reallocateCallback(void* userData, void* original, size_t size, size_t alignment,
									VkSystemAllocationScope allocationScope);

	static void freeCallback(void* userData, void* ptr);
#endif

	void resetFrame(PerFrame& frame);

	static VkBool32 debugReportCallbackEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
										   uint64_t object, size_t location, int32_t messageCode,
										   const char* pLayerPrefix, const char* pMessage, void* pUserData);

	ANKI_USE_RESULT Error printPipelineShaderInfoInternal(VkPipeline ppline, CString name, ShaderTypeBit stages,
														  U64 hash) const;
};
/// @}

} // end namespace anki
