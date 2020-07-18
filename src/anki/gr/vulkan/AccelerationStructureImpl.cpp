// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/AccelerationStructureImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

AccelerationStructureImpl::~AccelerationStructureImpl()
{
	if(m_handle)
	{
		vkDestroyAccelerationStructureKHR(getDevice(), m_handle, nullptr);
	}

	if(m_memHandle)
	{
		getGrManagerImpl().getGpuMemoryManager().freeMemory(m_memHandle);
	}
}

Error AccelerationStructureImpl::init(const AccelerationStructureInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());
	m_type = inf.m_type;

	// Create the handle
	{
		VkAccelerationStructureCreateGeometryTypeInfoKHR geom{};
		geom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
		geom.maxPrimitiveCount = 1;

		if(m_type == AccelerationStructureType::BOTTOM_LEVEL)
		{
			geom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			geom.indexType = convertIndexType(inf.m_bottomLevel.m_indexType);
			geom.vertexFormat = convertFormat(inf.m_bottomLevel.m_positionsFormat);
			geom.maxPrimitiveCount = inf.m_bottomLevel.m_indexCount / 3;
			geom.maxVertexCount = inf.m_bottomLevel.m_vertexCount;
			geom.allowsTransforms = false;
		}
		else
		{
			geom.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
			geom.maxPrimitiveCount = inf.m_topLevel.m_bottomLevelCount;
			geom.allowsTransforms = true;
		}

		VkAccelerationStructureCreateInfoKHR ci{};
		ci.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;

		ci.type = convertAccelerationStructureType(m_type);
		ci.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		ci.maxGeometryCount = 1;
		ci.pGeometryInfos = &geom;

		ANKI_VK_CHECK(vkCreateAccelerationStructureKHR(getDevice(), &ci, nullptr, &m_handle));
		getGrManagerImpl().trySetVulkanHandleName(
			inf.getName(), VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT, m_handle);
	}

	// Allocate memory
	{
		// Get mem requirements
		VkAccelerationStructureMemoryRequirementsInfoKHR reqIn{};
		reqIn.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
		reqIn.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
		reqIn.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		reqIn.accelerationStructure = m_handle;

		VkMemoryRequirements2 req{};
		req.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

		vkGetAccelerationStructureMemoryRequirementsKHR(getDevice(), &reqIn, &req);

		// Find mem IDX
		U32 memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// Fallback
		if(memIdx == MAX_U32)
		{
			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(
				req.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		}

		ANKI_ASSERT(memIdx != MAX_U32);

		// Allocate
		// TODO is it linear or no-linear?
		getGrManagerImpl().getGpuMemoryManager().allocateMemory(
			memIdx, req.memoryRequirements.size, U32(req.memoryRequirements.alignment), true, false, m_memHandle);

		// Bind memory
		VkBindAccelerationStructureMemoryInfoKHR bindInfo{};
		bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
		bindInfo.accelerationStructure = m_handle;
		bindInfo.memory = m_memHandle.m_memory;
		bindInfo.memoryOffset = m_memHandle.m_offset;
		ANKI_VK_CHECK(vkBindAccelerationStructureMemoryKHR(getDevice(), 1, &bindInfo));
	}

	return Error::NONE;
}

} // end namespace anki
