// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/vulkan/GpuMemoryAllocator.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Vulkan implementation of GrManager.
class GrManagerImpl
{
public:
	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	VkQueue m_queue = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_globalDescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout m_globalPipelineLayout = VK_NULL_HANDLE;

	GrManagerImpl(GrManager* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	~GrManagerImpl()
	{
	}

	ANKI_USE_RESULT Error init();

	/// Get or create a compatible render pass for a pipeline.
	VkRenderPass getOrCreateCompatibleRenderPass(const PipelineInitInfo& init);

	GrAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

private:
	GrManager* m_manager = nullptr;
	GrAllocator<U8> m_alloc;

	/// Map for compatible render passes.
	class CompatibleRenderPassHashMap;
	CompatibleRenderPassHashMap* m_renderPasses = nullptr;

	/// @name Memory
	/// @{
	VkPhysicalDeviceMemoryProperties m_memoryProperties;

	/// One for each mem type.
	DynamicArray<GpuMemoryAllocator> m_gpuMemAllocs;
	/// @}

	void initGlobalDsetLayout();
	void initGlobalPplineLayout();

	void initMemory();

	/// Find a suitable memory type.
	U findMemoryType(U resourceMemTypeBits,
		VkMemoryPropertyFlags preferFlags,
		VkMemoryPropertyFlags avoidFlags) const;
};
/// @}

} // end namespace anki
