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

	m_topLevelInfo.m_instances.destroy(getAllocator());
}

Error AccelerationStructureImpl::init(const AccelerationStructureInitInfo& inf)
{
	ANKI_TRACE_SCOPED_EVENT(VK_INIT_ACC_STRUCT);

	ANKI_ASSERT(inf.isValid());
	m_type = inf.m_type;

	if(m_type == AccelerationStructureType::BOTTOM_LEVEL)
	{
		static_cast<BottomLevelAccelerationStructureInitInfo&>(m_bottomLevelInfo) = inf.m_bottomLevel;
	}
	else
	{
		m_topLevelInfo.m_instances.create(getAllocator(), inf.m_topLevel.m_instances.getSize());
		for(U32 i = 0; i < inf.m_topLevel.m_instances.getSize(); ++i)
		{
			m_topLevelInfo.m_instances[i] = inf.m_topLevel.m_instances[i];
		}
	}

	// Create the handle
	{
		VkAccelerationStructureCreateGeometryTypeInfoKHR geom = {};
		geom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
		geom.maxPrimitiveCount = 1;

		if(m_type == AccelerationStructureType::BOTTOM_LEVEL)
		{
			geom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			geom.indexType = convertIndexType(m_bottomLevelInfo.m_indexType);
			geom.vertexFormat = convertFormat(m_bottomLevelInfo.m_positionsFormat);
			geom.maxPrimitiveCount = m_bottomLevelInfo.m_indexCount / 3;
			geom.maxVertexCount = m_bottomLevelInfo.m_positionCount;

			VkFormatProperties2 formatProperties = {};
			formatProperties.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
			vkGetPhysicalDeviceFormatProperties2(getGrManagerImpl().getPhysicalDevice(), geom.vertexFormat,
												 &formatProperties);

			if(!(formatProperties.formatProperties.bufferFeatures
				 & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR))
			{
				ANKI_VK_LOGE("The positions format cannot be used in acceleration structures");
				return Error::USER_DATA;
			}
		}
		else
		{
			geom.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
			geom.maxPrimitiveCount = m_topLevelInfo.m_instances.getSize();
		}

		geom.allowsTransforms = false; // Only for triangle types and at the moment not used

		VkAccelerationStructureCreateInfoKHR ci = {};
		ci.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;

		ci.type = convertAccelerationStructureType(m_type);
		ci.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		ci.maxGeometryCount = 1;
		ci.pGeometryInfos = &geom;

		ANKI_VK_CHECK(vkCreateAccelerationStructureKHR(getDevice(), &ci, nullptr, &m_handle));
		getGrManagerImpl().trySetVulkanHandleName(inf.getName(),
												  VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT, m_handle);
	}

	// Allocate memory
	{
		// Get mem requirements
		VkAccelerationStructureMemoryRequirementsInfoKHR reqIn = {};
		reqIn.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
		reqIn.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
		reqIn.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		reqIn.accelerationStructure = m_handle;

		VkMemoryRequirements2 req = {};
		req.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

		vkGetAccelerationStructureMemoryRequirementsKHR(getDevice(), &reqIn, &req);

		// Find mem IDX
		U32 memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryRequirements.memoryTypeBits,
																			 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
																			 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// Fallback
		if(memIdx == MAX_U32)
		{
			memIdx = getGrManagerImpl().getGpuMemoryManager().findMemoryType(req.memoryRequirements.memoryTypeBits,
																			 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		}

		ANKI_ASSERT(memIdx != MAX_U32);

		// Allocate
		// TODO is it linear or no-linear?
		getGrManagerImpl().getGpuMemoryManager().allocateMemory(
			memIdx, req.memoryRequirements.size, U32(req.memoryRequirements.alignment), true, m_memHandle);

		// Bind memory
		VkBindAccelerationStructureMemoryInfoKHR bindInfo = {};
		bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
		bindInfo.accelerationStructure = m_handle;
		bindInfo.memory = m_memHandle.m_memory;
		bindInfo.memoryOffset = m_memHandle.m_offset;
		ANKI_VK_CHECK(vkBindAccelerationStructureMemoryKHR(getDevice(), 1, &bindInfo));
	}

	// Get scratch buffer size
	{
		VkMemoryRequirements2 req = {};
		req.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

		VkAccelerationStructureMemoryRequirementsInfoKHR inf = {};
		inf.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
		inf.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
		inf.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		inf.accelerationStructure = m_handle;
		vkGetAccelerationStructureMemoryRequirementsKHR(getDevice(), &inf, &req);

		m_scratchBufferSize = U32(req.memoryRequirements.size);
	}

	// Get GPU address
	if(m_type == AccelerationStructureType::BOTTOM_LEVEL)
	{
		VkAccelerationStructureDeviceAddressInfoKHR inf = {};
		inf.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		inf.accelerationStructure = m_handle;

		m_bottomLevelInfo.m_gpuAddress = vkGetAccelerationStructureDeviceAddressKHR(getDevice(), &inf);
		ANKI_ASSERT(m_bottomLevelInfo.m_gpuAddress);
	}

	initBuildInfo();

	return Error::NONE;
}

void AccelerationStructureImpl::initBuildInfo()
{
	if(m_type == AccelerationStructureType::BOTTOM_LEVEL)
	{
		m_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		m_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		m_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR; // TODO

		VkAccelerationStructureGeometryTrianglesDataKHR& triangles = m_geometry.geometry.triangles;
		triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		triangles.vertexFormat = convertFormat(m_bottomLevelInfo.m_positionsFormat);
		triangles.vertexData.deviceAddress =
			m_bottomLevelInfo.m_positionBuffer->getGpuAddress() + m_bottomLevelInfo.m_positionBufferOffset;
		triangles.vertexStride = m_bottomLevelInfo.m_positionStride;
		triangles.indexType = convertIndexType(m_bottomLevelInfo.m_indexType);
		triangles.indexData.deviceAddress =
			m_bottomLevelInfo.m_indexBuffer->getGpuAddress() + m_bottomLevelInfo.m_indexBufferOffset;
	}
	else
	{
		const U32 instanceCount = m_topLevelInfo.m_instances.getSize();

		// Create the instances buffer
		{
			BufferInitInfo buffInit("RT_instances");
			buffInit.m_size = sizeof(VkAccelerationStructureInstanceKHR) * instanceCount;
			buffInit.m_usage = InternalBufferUsageBit::ACCELERATION_STRUCTURE_BUILD_SCRATCH;
			buffInit.m_mapAccess = BufferMapAccessBit::WRITE;
			m_topLevelInfo.m_instancesBuff = getManager().newBuffer(buffInit);
		}

		// Populate the instances buffer
		{
			VkAccelerationStructureInstanceKHR* instances = static_cast<VkAccelerationStructureInstanceKHR*>(
				m_topLevelInfo.m_instancesBuff->map(0, MAX_PTR_SIZE, BufferMapAccessBit::WRITE));
			for(U32 i = 0; i < instanceCount; ++i)
			{
				VkAccelerationStructureInstanceKHR& outInst = instances[i];
				const AccelerationStructureInstance& inInst = m_topLevelInfo.m_instances[i];
				static_assert(sizeof(outInst.transform) == sizeof(inInst.m_transform), "See file");
				memcpy(&outInst.transform, &inInst.m_transform, sizeof(inInst.m_transform));
				outInst.instanceCustomIndex = i & 0xFFFFFF;
				outInst.mask = inInst.m_mask;
				outInst.instanceShaderBindingTableRecordOffset = inInst.m_hitgroupSbtRecordIndex & 0xFFFFFF;
				outInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR
								| VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
				outInst.accelerationStructureReference =
					static_cast<const AccelerationStructureImpl&>(*inInst.m_bottomLevel).m_bottomLevelInfo.m_gpuAddress;
				ANKI_ASSERT(outInst.accelerationStructureReference != 0);
			}

			m_topLevelInfo.m_instancesBuff->unmap();
		}

		m_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		m_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		m_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR; // TODO

		VkAccelerationStructureGeometryInstancesDataKHR& inst = m_geometry.geometry.instances;
		inst.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		inst.arrayOfPointers = false;
		inst.data.deviceAddress = m_topLevelInfo.m_instancesBuff->getGpuAddress();
	}

	m_buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	m_buildInfo.type = convertAccelerationStructureType(m_type);
	m_buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	m_buildInfo.update = false;
	m_buildInfo.dstAccelerationStructure = m_handle;
	m_buildInfo.geometryArrayOfPointers = false;
	m_buildInfo.geometryCount = 1;
	m_buildInfo.ppGeometries = &m_geometryPtr;
	m_buildInfo.scratchData.deviceAddress = MAX_U64;

	// Ofset info
	m_offsetInfo.firstVertex = 0;
	if(m_type == AccelerationStructureType::BOTTOM_LEVEL)
	{
		m_offsetInfo.primitiveCount = m_bottomLevelInfo.m_indexCount / 3;
	}
	else
	{
		m_offsetInfo.primitiveCount = m_topLevelInfo.m_instances.getSize();
	}
	m_offsetInfo.primitiveOffset = 0;
	m_offsetInfo.transformOffset = 0;
}

void AccelerationStructureImpl::computeBarrierInfo(AccelerationStructureUsageBit before,
												   AccelerationStructureUsageBit after, VkPipelineStageFlags& srcStages,
												   VkAccessFlags& srcAccesses, VkPipelineStageFlags& dstStages,
												   VkAccessFlags& dstAccesses)
{
	// Before
	srcStages = 0;
	srcAccesses = 0;

	if(before == AccelerationStructureUsageBit::NONE)
	{
		srcStages |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		srcAccesses |= 0;
	}

	if(!!(before & AccelerationStructureUsageBit::BUILD))
	{
		srcStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		srcAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	}

	if(!!(before & AccelerationStructureUsageBit::ATTACH))
	{
		srcStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		srcAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(!!(before & AccelerationStructureUsageBit::GEOMETRY_READ))
	{
		srcStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		srcAccesses |= VK_ACCESS_MEMORY_READ_BIT; // READ_BIT is the only viable solution by elimination
	}

	if(!!(before & AccelerationStructureUsageBit::FRAGMENT_READ))
	{
		srcStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		srcAccesses |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(before & AccelerationStructureUsageBit::COMPUTE_READ))
	{
		srcStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		srcAccesses |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(before & AccelerationStructureUsageBit::TRACE_RAYS_READ))
	{
		srcStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		srcAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	// After
	dstStages = 0;
	dstAccesses = 0;

	if(!!(after & AccelerationStructureUsageBit::BUILD))
	{
		dstStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		dstAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	}

	if(!!(after & AccelerationStructureUsageBit::ATTACH))
	{
		dstStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		dstAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(!!(after & AccelerationStructureUsageBit::GEOMETRY_READ))
	{
		dstStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		dstAccesses |= VK_ACCESS_MEMORY_READ_BIT; // READ_BIT is the only viable solution by elimination
	}

	if(!!(after & AccelerationStructureUsageBit::FRAGMENT_READ))
	{
		dstStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dstAccesses |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(after & AccelerationStructureUsageBit::COMPUTE_READ))
	{
		dstStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstAccesses |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(after & AccelerationStructureUsageBit::TRACE_RAYS_READ))
	{
		dstStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		dstAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	ANKI_ASSERT(srcStages && dstStages);
}

} // end namespace anki
