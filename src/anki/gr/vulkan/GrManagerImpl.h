// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>
#include <anki/gr/vulkan/GrSemaphore.h>
#include <anki/gr/vulkan/Fence.h>
#include <anki/gr/vulkan/QueryExtra.h>
#include <anki/gr/vulkan/DescriptorSet.h>
#include <anki/gr/vulkan/CommandBufferExtra.h>
#include <anki/gr/vulkan/PipelineLayout.h>
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
class GrManagerImpl
{
public:
	GrManagerImpl(GrManager* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	~GrManagerImpl();

	ANKI_USE_RESULT Error init(const GrManagerInitInfo& cfg);

	GrAllocator<U8> getAllocator() const;

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

	VkCommandBuffer newCommandBuffer(ThreadId tid, CommandBufferFlag cmdbFlags);

	void deleteCommandBuffer(VkCommandBuffer cmdb, CommandBufferFlag cmdbFlags, ThreadId tid);

	FencePtr newFence()
	{
		return m_fences.newInstance();
	}
	/// @}

	VkFormat getDefaultFramebufferSurfaceFormat() const
	{
		return m_surfaceFormat;
	}

	VkImageView getDefaultSurfaceImageView(U idx) const
	{
		ANKI_ASSERT(m_backbuffers[idx].m_imageView);
		return m_backbuffers[idx].m_imageView;
	}

	VkImage getDefaultSurfaceImage(U idx) const
	{
		ANKI_ASSERT(m_backbuffers[idx].m_image);
		return m_backbuffers[idx].m_image;
	}

	U getCurrentBackbufferIndex() const
	{
		return m_crntBackbufferIdx;
	}

	U getDefaultSurfaceWidth() const
	{
		ANKI_ASSERT(m_surfaceWidth);
		return m_surfaceWidth;
	}

	U getDefaultSurfaceHeight() const
	{
		ANKI_ASSERT(m_surfaceHeight);
		return m_surfaceHeight;
	}

	void flushCommandBuffer(CommandBufferPtr ptr, Bool wait = false);

	void finishCommandBuffer(CommandBufferPtr ptr)
	{
		flushCommandBuffer(ptr, true);
	}

	/// @name Memory
	/// @{
	GpuMemoryManager& getGpuMemoryManager()
	{
		return m_gpuMemManager;
	}

	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const
	{
		return m_memoryProperties;
	}
	/// @}

	GpuVendor getGpuVendor() const
	{
		return m_vendor;
	}

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

	GrObjectCache& getSamplerCache()
	{
		ANKI_ASSERT(m_samplerCache);
		return *m_samplerCache;
	}

	DescriptorSetFactory& getDescriptorSetFactory()
	{
		return m_descrFactory;
	}

	VkPipelineCache getPipelineCache() const
	{
		ANKI_ASSERT(m_pplineCache);
		return m_pplineCache;
	}

	PipelineLayoutFactory& getPipelineLayoutFactory()
	{
		return m_pplineLayoutFactory;
	}

	VulkanExtensions getExtensions() const
	{
		return m_extensions;
	}

	void trySetVulkanHandleName(CString name, VkDebugReportObjectTypeEXT type, U64 handle) const;

private:
	GrManager* m_manager = nullptr;

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
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VulkanExtensions m_extensions = VulkanExtensions::NONE;
	GpuVendor m_vendor = GpuVendor::UNKNOWN;
	VkDevice m_device = VK_NULL_HANDLE;
	U32 m_queueIdx = MAX_U32;
	VkQueue m_queue = VK_NULL_HANDLE;
	Mutex m_globalMtx;

	VkPhysicalDeviceProperties m_devProps = {};
	VkPhysicalDeviceFeatures m_devFeatures = {};

	PFN_vkDebugMarkerSetObjectNameEXT m_pfnDebugMarkerSetObjectNameEXT = nullptr;

	/// @name Surface_related
	/// @{
	class Backbuffer
	{
	public:
		VkImage m_image = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
	};

	class PerFrame
	{
	public:
		FencePtr m_presentFence;
		GrSemaphorePtr m_acquireSemaphore;

		/// The semaphore that the submit that renders to the default FB.
		GrSemaphorePtr m_renderSemaphore;

		/// Keep it here for deferred cleanup.
		List<CommandBufferPtr> m_cmdbsSubmitted;
	};

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	U32 m_surfaceWidth = 0, m_surfaceHeight = 0;
	VkFormat m_surfaceFormat = VK_FORMAT_UNDEFINED;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	Array<PerFrame, MAX_FRAMES_IN_FLIGHT> m_perFrame;
	Array<Backbuffer, MAX_FRAMES_IN_FLIGHT> m_backbuffers;
	U32 m_crntBackbufferIdx = 0;
	/// @}

	/// @name Memory
	/// @{
	VkPhysicalDeviceMemoryProperties m_memoryProperties;

	/// The main allocator.
	GpuMemoryManager m_gpuMemManager;
	/// @}

	/// @name Per_thread_cache
	/// @{

	class PerThreadHasher
	{
	public:
		U64 operator()(const ThreadId& b) const
		{
			return b;
		}
	};

	class PerThreadCompare
	{
	public:
		Bool operator()(const ThreadId& a, const ThreadId& b) const
		{
			return a == b;
		}
	};

	/// Per thread cache.
	class PerThread
	{
	public:
		CommandBufferFactory m_cmdbs;
	};

	HashMap<ThreadId, PerThread, PerThreadHasher, PerThreadCompare> m_perThread;
	SpinLock m_perThreadMtx;

	FenceFactory m_fences;
	GrSemaphoreFactory m_semaphores;
	/// @}

	PipelineLayoutFactory m_pplineLayoutFactory;

	DescriptorSetFactory m_descrFactory;

	QueryAllocator m_queryAlloc;

	VkPipelineCache m_pplineCache = VK_NULL_HANDLE;

	Bool8 m_r8g8b8ImagesSupported = false;
	Bool8 m_s8ImagesSupported = false;
	Bool8 m_d24S8ImagesSupported = false;

	GrObjectCache* m_samplerCache = nullptr;

	ANKI_USE_RESULT Error initInternal(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initInstance(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initSurface(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initDevice(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initSwapchain(const GrManagerInitInfo& init);
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

	PerThread& getPerThreadCache(ThreadId tid);
};
/// @}

} // end namespace anki
