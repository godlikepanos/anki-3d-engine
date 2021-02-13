// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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

	void generateBuildInfo(U64 scratchBufferAddress, VkAccelerationStructureBuildGeometryInfoKHR& buildInfo,
						   VkAccelerationStructureBuildRangeInfoKHR& rangeInfo) const
	{
		buildInfo = m_buildInfo;
		buildInfo.scratchData.deviceAddress = scratchBufferAddress;
		rangeInfo = m_rangeInfo;
	}

	static void computeBarrierInfo(AccelerationStructureUsageBit before, AccelerationStructureUsageBit after,
								   VkPipelineStageFlags& srcStages, VkAccessFlags& srcAccesses,
								   VkPipelineStageFlags& dstStages, VkAccessFlags& dstAccesses);

private:
	class ASBottomLevelInfo
	{
	public:
		BufferPtr m_positionsBuffer;
		BufferPtr m_indexBuffer;
	};

	class ASTopLevelInfo
	{
	public:
		BufferPtr m_instancesBuffer;
		DynamicArray<AccelerationStructurePtr> m_blas;
	};

	BufferPtr m_asBuffer;
	VkAccelerationStructureKHR m_handle = VK_NULL_HANDLE;
	VkDeviceAddress m_deviceAddress = 0;

	ASBottomLevelInfo m_bottomLevelInfo;
	ASTopLevelInfo m_topLevelInfo;

	/// @name Build-time info
	/// @{
	VkAccelerationStructureGeometryKHR m_geometry = {};
	VkAccelerationStructureBuildGeometryInfoKHR m_buildInfo = {};
	VkAccelerationStructureBuildRangeInfoKHR m_rangeInfo = {};
	U32 m_scratchBufferSize = 0;
	/// @}
};
/// @}

} // end namespace anki
