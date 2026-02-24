// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Gr/Vulkan/VkSemaphoreFactory.h>
#include <AnKi/Gr/Vulkan/VkFenceFactory.h>
#include <AnKi/Gr/Vulkan/VkSwapchainFactory.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/BlockArray.h>

namespace anki {

// Note: Disable that because it crashes Intel drivers
#define ANKI_GR_MANAGER_DEBUG_MEMMORY ANKI_EXTRA_CHECKS && 0

// Forward
class TextureFallbackUploader;
class MicroCommandBuffer;

enum class AsyncComputeType
{
	kProper,
	kLowPriorityQueue,
	kDisabled
};

// A small struct with all the caps we need.
class VulkanCapabilities
{
public:
	VkDeviceAddress m_nonCoherentAtomSize = 0;
	U64 m_maxTexelBufferElements = 0;
	F32 m_timestampPeriod = 0.0f;
	U32 m_asBuildScratchAlignment = 0;
	U32 m_asBufferAlignment = 256; // Spec says 256
};

// Vulkan implementation of GrManager.
class GrManagerImpl : public GrManager
{
	friend class GrManager;

public:
	GrManagerImpl()
	{
	}

	~GrManagerImpl();

	Error init(const GrManagerInitInfo& cfg);

	ConstWeakArray<U32> getQueueFamilies() const
	{
		const Bool hasAsyncCompute = m_queueFamilyIndices[GpuQueueType::kGeneral] != m_queueFamilyIndices[GpuQueueType::kCompute];
		return (hasAsyncCompute) ? m_queueFamilyIndices : ConstWeakArray<U32>(&m_queueFamilyIndices[0], 1);
	}

	AsyncComputeType getAsyncComputeType() const
	{
		if(m_queues[GpuQueueType::kCompute] == nullptr)
		{
			return AsyncComputeType::kDisabled;
		}
		else if(m_queueFamilyIndices[GpuQueueType::kCompute] == m_queueFamilyIndices[GpuQueueType::kGeneral])
		{
			return AsyncComputeType::kLowPriorityQueue;
		}
		else
		{
			return AsyncComputeType::kProper;
		}
	}

	const VulkanCapabilities& getVulkanCapabilities() const
	{
		return m_caps;
	}

	TexturePtr acquireNextPresentableTexture();

	void beginFrameInternal();

	void endFrameInternal();

	void submitInternal(WeakArray<CommandBuffer*> cmdbs, WeakArray<Fence*> waitFences, FencePtr* signalFence, Bool flushAndSerialize);

	void finishInternal();

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

	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const
	{
		return m_memoryProperties;
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

	void getNativeWindowSize(U32& width, U32& height) const
	{
		width = m_nativeWindowWidth;
		height = m_nativeWindowHeight;
		ANKI_ASSERT(width && height);
	}

	// Debug report //
	void trySetVulkanHandleName(CString name, VkObjectType type, U64 handle) const;

	void trySetVulkanHandleName(CString name, VkObjectType type, void* handle) const
	{
		trySetVulkanHandleName(name, type, U64(ptrToNumber(handle)));
	}
	// End debug report //

	// Node: It's thread-safe.
	void printPipelineShaderInfo(VkPipeline ppline, CString name, U64 hash = 0) const;

	// Node: It's thread-safe.
	void releaseObject(GrObject* object)
	{
		ANKI_ASSERT(object);
		LockGuard lock(m_globalMtx);
		releaseObjectDeleteLoop(object);
	}

	void releaseObjectDeleteLoop(GrObject* object);

	// Find a suitable memory type.
	U32 findMemoryType(U32 resourceMemTypeBits, VkMemoryPropertyFlags preferFlags, VkMemoryPropertyFlags avoidFlags) const;

private:
	// Contains objects that are marked for deletion
	class CleanupGroup
	{
	public:
		Array<MicroFencePtr, U32(GpuQueueType::kCount)> m_fences;
		GrDynamicArray<GrObject*> m_objectsMarkedForDeletion;
		Bool m_finalized = false;

		~CleanupGroup()
		{
			ANKI_ASSERT(m_objectsMarkedForDeletion.getSize() == 0);
		}

		Bool canDelete() const
		{
			if(m_fences[GpuQueueType::kGeneral] && !m_fences[GpuQueueType::kGeneral]->signaled())
			{
				return false;
			}

			if(m_fences[GpuQueueType::kCompute] && !m_fences[GpuQueueType::kCompute]->signaled())
			{
				return false;
			}

			return true;
		}
	};

	class AcquireData
	{
	public:
		MicroSemaphorePtr m_semaphore;
		MicroFencePtr m_fence; // It's used to check if we can delete the semaphore

		~AcquireData()
		{
			ANKI_ASSERT(!m_fence || m_fence->signaled());
		}
	};

	U64 m_frame = 0;

	GrBlockArray<CleanupGroup> m_cleanupGroups;
	U32 m_activeCleanupGroup = kMaxU32;

	Array<MicroFencePtr, U32(GpuQueueType::kCount) + 1> m_crntFences;

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	U32 m_nativeWindowWidth = 0;
	U32 m_nativeWindowHeight = 0;
	MicroSwapchainPtr m_crntSwapchain;

	GrBlockArray<AcquireData> m_acquireData;
	U32 m_activeAcquireDataIdx = kMaxU32;
	GpuQueueType m_queueWroteToSwapchainImage = GpuQueueType::kCount;
	U8 m_acquiredImageIdx = kMaxU8;

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
	Array<VkQueue, U32(GpuQueueType::kCount)> m_queues = {nullptr, nullptr};
	Mutex m_globalMtx;

	VkDebugUtilsMessengerEXT m_debugUtilsMessager = VK_NULL_HANDLE;

	mutable SpinLock m_shaderStatsMtx;

	VulkanCapabilities m_caps;

	VkPhysicalDeviceMemoryProperties m_memoryProperties;

	Error initInternal(const GrManagerInitInfo& init);
	Error initInstance();
	Error initSurface();
	Error initDevice();
	Error initMemory();

#if ANKI_GR_MANAGER_DEBUG_MEMMORY
	static void* allocateCallback(void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);

	static void* reallocateCallback(void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);

	static void freeCallback(void* userData, void* ptr);
#endif

	static VkBool32 debugReportCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
										   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	Error printPipelineShaderInfoInternal(VkPipeline ppline, CString name, U64 hash) const;

	template<typename TProps>
	void getPhysicalDeviceProperties2(TProps& props) const
	{
		VkPhysicalDeviceProperties2 properties = {};
		properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		properties.pNext = &props;
		vkGetPhysicalDeviceProperties2(m_physicalDevice, &properties);
	}

	template<typename TFeatures>
	void getPhysicalDevicaFeatures2(TFeatures& features) const
	{
		VkPhysicalDeviceFeatures2 features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &features;
		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features2);
	}

	void deleteObjectsMarkedForDeletion();
};

} // end namespace anki
