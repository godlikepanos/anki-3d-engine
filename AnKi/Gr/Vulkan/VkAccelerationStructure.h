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
	friend class AccelerationStructure;

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

	void generateBuildInfo(U64 scratchBufferAddress, VkAccelerationStructureBuildGeometryInfoKHR& buildInfo,
						   VkAccelerationStructureBuildRangeInfoKHR& rangeInfo) const;

	static VkMemoryBarrier computeBarrierInfo(AccelerationStructureUsageBit before, AccelerationStructureUsageBit after,
											  VkPipelineStageFlags& srcStages, VkPipelineStageFlags& dstStages);

	static void getMemoryRequirement(const AccelerationStructureInitInfo& init, PtrSize& asBufferSize, PtrSize& buildScratchBufferSize);

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
	};

	BufferInternalPtr m_asBuffer;
	PtrSize m_asBufferOffset = kMaxPtrSize;

	VkAccelerationStructureKHR m_handle = VK_NULL_HANDLE;
	VkDeviceAddress m_deviceAddress = 0;

	ASBottomLevelInfo m_blas;
	ASTopLevelInfo m_tlas;

	/// @name Build-time info
	/// @{
	VkAccelerationStructureGeometryKHR m_geometry = {};
	VkAccelerationStructureBuildGeometryInfoKHR m_buildInfo = {};
	VkAccelerationStructureBuildRangeInfoKHR m_rangeInfo = {};
	/// @}
};
/// @}

} // end namespace anki
