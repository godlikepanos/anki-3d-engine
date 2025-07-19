// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/AccelerationStructure.h>
#include <AnKi/Gr/Vulkan/VkCommon.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// AccelerationStructure implementation.
class AccelerationStructureImpl final : public AccelerationStructure
{
public:
	AccelerationStructureImpl(CString name)
		: AccelerationStructure(name)
	{
	}

	~AccelerationStructureImpl();

	Error init(const AccelerationStructureInitInfo& inf);

	const VkAccelerationStructureKHR& getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	U32 getMaxInstanceCount() const
	{
		ANKI_ASSERT(m_topLevelInfo.m_maxInstanceCount);
		return m_topLevelInfo.m_maxInstanceCount;
	}

	VkDeviceAddress getAsDeviceAddress() const
	{
		ANKI_ASSERT(m_deviceAddress);
		return m_deviceAddress;
	}

	void generateBuildInfo(U64 scratchBufferAddress, VkAccelerationStructureBuildGeometryInfoKHR& buildInfo,
						   VkAccelerationStructureBuildRangeInfoKHR& rangeInfo) const
	{
		buildInfo = m_buildInfo;
		buildInfo.scratchData.deviceAddress = scratchBufferAddress;
		rangeInfo = m_rangeInfo;
	}

	static VkMemoryBarrier computeBarrierInfo(AccelerationStructureUsageBit before, AccelerationStructureUsageBit after,
											  VkPipelineStageFlags& srcStages, VkPipelineStageFlags& dstStages);

private:
	class ASBottomLevelInfo
	{
	public:
		BufferInternalPtr m_positionsBuffer;
		BufferInternalPtr m_indexBuffer;
	};

	class ASTopLevelInfo
	{
	public:
		BufferInternalPtr m_instancesBuffer;
		GrDynamicArray<AccelerationStructureInternalPtr> m_blases;
		U32 m_maxInstanceCount = 0; ///< Only for indirect.
	};

	BufferInternalPtr m_asBuffer;
	VkAccelerationStructureKHR m_handle = VK_NULL_HANDLE;
	VkDeviceAddress m_deviceAddress = 0;

	ASBottomLevelInfo m_bottomLevelInfo;
	ASTopLevelInfo m_topLevelInfo;

	/// @name Build-time info
	/// @{
	VkAccelerationStructureGeometryKHR m_geometry = {};
	VkAccelerationStructureBuildGeometryInfoKHR m_buildInfo = {};
	VkAccelerationStructureBuildRangeInfoKHR m_rangeInfo = {};
	/// @}
};
/// @}

} // end namespace anki
