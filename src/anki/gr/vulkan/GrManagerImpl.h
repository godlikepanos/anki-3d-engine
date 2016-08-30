// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/vulkan/GpuMemoryAllocator.h>
#include <anki/gr/vulkan/CommandBufferInternal.h>
#include <anki/gr/vulkan/Semaphore.h>
#include <anki/gr/vulkan/Fence.h>
#include <anki/gr/vulkan/TransientMemoryManager.h>
#include <anki/gr/vulkan/QueryAllocator.h>
#include <anki/gr/vulkan/CompatibleRenderPassCreator.h>
#include <anki/util/HashMap.h>

namespace anki
{

// Forward
class TextureFallbackUploader;

/// @addtogroup vulkan
/// @{

/// Vulkan implementation of GrManager.
class GrManagerImpl
{
public:
	GrManagerImpl(GrManager* manager)
		: m_manager(manager)
		, m_rpCreator(this)
		, m_transientMem(manager)
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

	VkPipelineLayout getGlobalPipelineLayout() const
	{
		ANKI_ASSERT(m_globalPipelineLayout);
		return m_globalPipelineLayout;
	}

	/// @name object_creation
	/// @{

	ANKI_USE_RESULT Error allocateDescriptorSet(VkDescriptorSet& out)
	{
		out = VK_NULL_HANDLE;
		VkDescriptorSetAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ci.descriptorPool = m_globalDescriptorPool;
		ci.descriptorSetCount = 1;
		ci.pSetLayouts = &m_globalDescriptorSetLayout;

		LockGuard<Mutex> lock(m_globalDescriptorPoolMtx);
		if(++m_descriptorSetAllocationCount > MAX_RESOURCE_GROUPS)
		{
			ANKI_LOGE("Exceeded the MAX_RESOURCE_GROUPS");
			return ErrorCode::OUT_OF_MEMORY;
		}
		ANKI_VK_CHECK(vkAllocateDescriptorSets(m_device, &ci, &out));
		return ErrorCode::NONE;
	}

	void freeDescriptorSet(VkDescriptorSet ds)
	{
		ANKI_ASSERT(ds);
		LockGuard<Mutex> lock(m_globalDescriptorPoolMtx);
		--m_descriptorSetAllocationCount;
		ANKI_VK_CHECKF(vkFreeDescriptorSets(m_device, m_globalDescriptorPool, 1, &ds));
	}

	VkCommandBuffer newCommandBuffer(ThreadId tid, Bool secondLevel);

	void deleteCommandBuffer(VkCommandBuffer cmdb, Bool secondLevel, ThreadId tid);

	SemaphorePtr newSemaphore()
	{
		return m_semaphores.newInstance();
	}

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
		ANKI_ASSERT(m_perFrame[idx].m_imageView);
		return m_perFrame[idx].m_imageView;
	}

	VkImage getDefaultSurfaceImage(U idx) const
	{
		ANKI_ASSERT(m_perFrame[idx].m_image);
		return m_perFrame[idx].m_image;
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

	U64 getFrame() const
	{
		return m_frame;
	}

	void flushCommandBuffer(CommandBufferPtr ptr,
		SemaphorePtr signalSemaphore = {},
		WeakArray<SemaphorePtr> waitSemaphores = {},
		WeakArray<VkPipelineStageFlags> waitPplineStages = {});

	/// @name Memory
	/// @{
	GpuMemoryAllocator& getGpuMemoryAllocator()
	{
		return m_gpuAlloc;
	}

	TransientMemoryManager& getTransientMemoryManager()
	{
		return m_transientMem;
	}

	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const
	{
		return m_memoryProperties;
	}
	/// @}

	TextureFallbackUploader& getTextureFallbackUploader()
	{
		ANKI_ASSERT(m_texUploader);
		return *m_texUploader;
	}

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

	CompatibleRenderPassCreator& getCompatibleRenderPassCreator()
	{
		return m_rpCreator;
	}

private:
	GrManager* m_manager = nullptr;

	U64 m_frame = 0;

	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	GpuVendor m_vendor = GpuVendor::UNKNOWN;
	VkDevice m_device = VK_NULL_HANDLE;
	U32 m_queueIdx = MAX_U32;
	VkQueue m_queue = VK_NULL_HANDLE;
	Mutex m_queueSubmitMtx;

	VkPhysicalDeviceProperties m_devProps = {};
	VkPhysicalDeviceFeatures m_devFeatures = {};

	/// @name Surface_related
	/// @{
	class PerFrame
	{
	public:
		VkImage m_image = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;

		FencePtr m_presentFence;
		SemaphorePtr m_acquireSemaphore;

		/// The semaphore that the submit that renders to the default FB.
		SemaphorePtr m_renderSemaphore;

		/// Keep it here for deferred cleanup.
		List<CommandBufferPtr> m_cmdbsSubmitted;
	};

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	U32 m_surfaceWidth = 0, m_surfaceHeight = 0;
	VkFormat m_surfaceFormat = VK_FORMAT_UNDEFINED;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	Array<PerFrame, MAX_FRAMES_IN_FLIGHT> m_perFrame;
	/// @}

	VkDescriptorSetLayout m_globalDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_globalDescriptorPool = VK_NULL_HANDLE;
	Mutex m_globalDescriptorPoolMtx;
	U32 m_descriptorSetAllocationCount = 0;
	VkPipelineLayout m_globalPipelineLayout = VK_NULL_HANDLE;

	/// Map for compatible render passes.
	CompatibleRenderPassCreator m_rpCreator;

	/// @name Memory
	/// @{
	VkPhysicalDeviceMemoryProperties m_memoryProperties;

	/// The main allocator.
	GpuMemoryAllocator m_gpuAlloc;

	TransientMemoryManager m_transientMem;
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
	SemaphoreFactory m_semaphores;
	/// @}

	TextureFallbackUploader* m_texUploader = nullptr;

	QueryAllocator m_queryAlloc;

	Bool8 m_r8g8b8ImagesSupported = false;

	ANKI_USE_RESULT Error initInternal(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initInstance(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initSurface(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initDevice(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initSwapchain(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initFramebuffers(const GrManagerInitInfo& init);
	ANKI_USE_RESULT Error initGlobalDsetLayout();
	ANKI_USE_RESULT Error initGlobalDsetPool();
	ANKI_USE_RESULT Error initGlobalPplineLayout();
	ANKI_USE_RESULT Error initMemory(const ConfigSet& cfg);

	static void* allocateCallback(
		void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);

	static void* reallocateCallback(
		void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);

	static void freeCallback(void* userData, void* ptr);

	void resetFrame(PerFrame& frame);

	PerThread& getPerThreadCache(ThreadId tid);
};
/// @}

} // end namespace anki
