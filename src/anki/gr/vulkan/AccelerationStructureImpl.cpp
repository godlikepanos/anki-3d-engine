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
	ANKI_ASSERT(!"TODO");
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
		// TODO
	}

	return Error::NONE;
}

} // end namespace anki
