// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/AccelerationStructure.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// AccelerationStructure implementation.
class AccelerationStructureImpl final :
	public AccelerationStructure,
	public VulkanObject<AccelerationStructure, AccelerationStructureImpl>
{
public:
	AccelerationStructureImpl(GrManager* manager, CString name)
		: AccelerationStructure(manager, name)
	{
	}

	~AccelerationStructureImpl();

	ANKI_USE_RESULT Error init(const AccelerationStructureInitInfo& inf);

	VkAccelerationStructureKHR getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	U32 getBuildScratchBufferSize() const
	{
		ANKI_ASSERT(m_scratchBufferSize > 0);
		return m_scratchBufferSize;
	}

	void generateBuildInfo(U64 scratchBufferAddress, VkAccelerationStructureBuildGeometryInfoKHR& info,
						   VkAccelerationStructureBuildOffsetInfoKHR& offsetInfo) const
	{
		info = m_buildInfo;
		info.scratchData.deviceAddress = scratchBufferAddress;
		offsetInfo = m_offsetInfo;
	}

	static void computeBarrierInfo(AccelerationStructureUsageBit before, AccelerationStructureUsageBit after,
								   VkPipelineStageFlags& srcStages, VkAccessFlags& srcAccesses,
								   VkPipelineStageFlags& dstStages, VkAccessFlags& dstAccesses);

private:
	class ASBottomLevelInfo : public BottomLevelAccelerationStructureInitInfo
	{
	public:
		U64 m_gpuAddress = 0;
	};

	class ASTopLevelInfo
	{
	public:
		DynamicArray<AccelerationStructureInstance> m_instances;
		BufferPtr m_instancesBuff;
	};

	VkAccelerationStructureKHR m_handle = VK_NULL_HANDLE;
	GpuMemoryHandle m_memHandle;

	ASBottomLevelInfo m_bottomLevelInfo;
	ASTopLevelInfo m_topLevelInfo;

	U32 m_scratchBufferSize = 0;

	VkAccelerationStructureBuildGeometryInfoKHR m_buildInfo = {};
	VkAccelerationStructureGeometryKHR m_geometry = {};
	VkAccelerationStructureGeometryKHR* m_geometryPtr = &m_geometry;
	VkAccelerationStructureBuildOffsetInfoKHR m_offsetInfo = {};

	void initBuildInfo();
};
/// @}

} // end namespace anki
