// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/AccelerationStructureImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

namespace anki {

AccelerationStructureImpl::~AccelerationStructureImpl()
{
	m_topLevelInfo.m_blas.destroy(getMemoryPool());

	if(m_handle)
	{
		vkDestroyAccelerationStructureKHR(getDevice(), m_handle, nullptr);
	}
}

Error AccelerationStructureImpl::init(const AccelerationStructureInitInfo& inf)
{
	ANKI_TRACE_SCOPED_EVENT(VK_INIT_ACC_STRUCT);

	ANKI_ASSERT(inf.isValid());
	m_type = inf.m_type;

	if(m_type == AccelerationStructureType::kBottomLevel)
	{
		// Geom
		VkAccelerationStructureGeometryKHR& geom = m_geometry;
		geom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geom.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		geom.geometry.triangles.vertexFormat = convertFormat(inf.m_bottomLevel.m_positionsFormat);
		geom.geometry.triangles.vertexData.deviceAddress =
			inf.m_bottomLevel.m_positionBuffer->getGpuAddress() + inf.m_bottomLevel.m_positionBufferOffset;
		geom.geometry.triangles.vertexStride = inf.m_bottomLevel.m_positionStride;
		geom.geometry.triangles.maxVertex = inf.m_bottomLevel.m_positionCount - 1;
		geom.geometry.triangles.indexType = convertIndexType(inf.m_bottomLevel.m_indexType);
		geom.geometry.triangles.indexData.deviceAddress =
			inf.m_bottomLevel.m_indexBuffer->getGpuAddress() + inf.m_bottomLevel.m_indexBufferOffset;
		geom.flags = VK_GEOMETRY_OPAQUE_BIT_KHR; // TODO

		// Geom build info
		VkAccelerationStructureBuildGeometryInfoKHR& buildInfo = m_buildInfo;
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geom;

		// Get memory info
		VkAccelerationStructureBuildSizesInfoKHR buildSizes = {};
		const U32 primitiveCount = inf.m_bottomLevel.m_indexCount / 3;
		buildSizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
												&buildInfo, &primitiveCount, &buildSizes);
		m_scratchBufferSize = U32(buildSizes.buildScratchSize);

		// Create the buffer that holds the AS memory
		BufferInitInfo bufferInit(inf.getName());
		bufferInit.m_usage = PrivateBufferUsageBit::kAccelerationStructure;
		bufferInit.m_size = buildSizes.accelerationStructureSize;
		m_asBuffer = getManager().newBuffer(bufferInit);

		// Create the AS
		VkAccelerationStructureCreateInfoKHR asCi = {};
		asCi.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCi.createFlags = 0;
		asCi.buffer = static_cast<const BufferImpl&>(*m_asBuffer).getHandle();
		asCi.offset = 0;
		asCi.size = buildSizes.accelerationStructureSize;
		asCi.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		ANKI_VK_CHECK(vkCreateAccelerationStructureKHR(getDevice(), &asCi, nullptr, &m_handle));

		// Get its address
		VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {};
		addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		addressInfo.accelerationStructure = m_handle;
		m_deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(getDevice(), &addressInfo);

		// Almost finalize the build info
		buildInfo.dstAccelerationStructure = m_handle;

		// Range info
		m_rangeInfo.primitiveCount = primitiveCount;
	}
	else
	{
		// Create the instances buffer
		m_topLevelInfo.m_blas.resizeStorage(getMemoryPool(), inf.m_topLevel.m_instances.getSize());

		BufferInitInfo buffInit("AS instances");
		buffInit.m_size = sizeof(VkAccelerationStructureInstanceKHR) * inf.m_topLevel.m_instances.getSize();
		buffInit.m_usage = PrivateBufferUsageBit::kAccelerationStructure;
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		m_topLevelInfo.m_instancesBuffer = getManager().newBuffer(buffInit);

		VkAccelerationStructureInstanceKHR* instances = static_cast<VkAccelerationStructureInstanceKHR*>(
			m_topLevelInfo.m_instancesBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));
		for(U32 i = 0; i < inf.m_topLevel.m_instances.getSize(); ++i)
		{
			VkAccelerationStructureInstanceKHR& outInst = instances[i];
			const AccelerationStructureInstance& inInst = inf.m_topLevel.m_instances[i];
			static_assert(sizeof(outInst.transform) == sizeof(inInst.m_transform), "See file");
			memcpy(&outInst.transform, &inInst.m_transform, sizeof(inInst.m_transform));
			outInst.instanceCustomIndex = i & 0xFFFFFF;
			outInst.mask = inInst.m_mask;
			outInst.instanceShaderBindingTableRecordOffset = inInst.m_hitgroupSbtRecordIndex & 0xFFFFFF;
			outInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR
							| VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			outInst.accelerationStructureReference =
				static_cast<const AccelerationStructureImpl&>(*inInst.m_bottomLevel).m_deviceAddress;
			ANKI_ASSERT(outInst.accelerationStructureReference != 0);

			// Hold the reference
			m_topLevelInfo.m_blas.emplaceBack(getMemoryPool(), inf.m_topLevel.m_instances[i].m_bottomLevel);
		}

		m_topLevelInfo.m_instancesBuffer->flush(0, kMaxPtrSize);
		m_topLevelInfo.m_instancesBuffer->unmap();

		// Geom
		VkAccelerationStructureGeometryKHR& geom = m_geometry;
		geom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geom.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geom.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		geom.geometry.instances.data.deviceAddress = m_topLevelInfo.m_instancesBuffer->getGpuAddress();
		geom.geometry.instances.arrayOfPointers = false;
		geom.flags = VK_GEOMETRY_OPAQUE_BIT_KHR; // TODO

		// Geom build info
		VkAccelerationStructureBuildGeometryInfoKHR& buildInfo = m_buildInfo;
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geom;

		// Get memory info
		VkAccelerationStructureBuildSizesInfoKHR buildSizes = {};
		const U32 instanceCount = inf.m_topLevel.m_instances.getSize();
		buildSizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
												&buildInfo, &instanceCount, &buildSizes);
		m_scratchBufferSize = U32(buildSizes.buildScratchSize);

		// Create the buffer that holds the AS memory
		BufferInitInfo bufferInit(inf.getName());
		bufferInit.m_usage = PrivateBufferUsageBit::kAccelerationStructure;
		bufferInit.m_size = buildSizes.accelerationStructureSize;
		m_asBuffer = getManager().newBuffer(bufferInit);

		// Create the AS
		VkAccelerationStructureCreateInfoKHR asCi = {};
		asCi.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCi.createFlags = 0;
		asCi.buffer = static_cast<const BufferImpl&>(*m_asBuffer).getHandle();
		asCi.offset = 0;
		asCi.size = buildSizes.accelerationStructureSize;
		asCi.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		ANKI_VK_CHECK(vkCreateAccelerationStructureKHR(getDevice(), &asCi, nullptr, &m_handle));

		// Almost finalize the build info
		buildInfo.dstAccelerationStructure = m_handle;

		// Range info
		m_rangeInfo.primitiveCount = inf.m_topLevel.m_instances.getSize();
	}

	return Error::kNone;
}

void AccelerationStructureImpl::computeBarrierInfo(AccelerationStructureUsageBit before,
												   AccelerationStructureUsageBit after, VkPipelineStageFlags& srcStages,
												   VkAccessFlags& srcAccesses, VkPipelineStageFlags& dstStages,
												   VkAccessFlags& dstAccesses)
{
	// Before
	srcStages = 0;
	srcAccesses = 0;

	if(before == AccelerationStructureUsageBit::kNone)
	{
		srcStages |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		srcAccesses |= 0;
	}

	if(!!(before & AccelerationStructureUsageBit::kBuild))
	{
		srcStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		srcAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	}

	if(!!(before & AccelerationStructureUsageBit::kAttach))
	{
		srcStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		srcAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(!!(before & AccelerationStructureUsageBit::kGeometryRead))
	{
		srcStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		srcAccesses |= VK_ACCESS_MEMORY_READ_BIT; // READ_BIT is the only viable solution by elimination
	}

	if(!!(before & AccelerationStructureUsageBit::kFragmentRead))
	{
		srcStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		srcAccesses |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(before & AccelerationStructureUsageBit::kComputeRead))
	{
		srcStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		srcAccesses |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(before & AccelerationStructureUsageBit::kTraceRaysRead))
	{
		srcStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		srcAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	// After
	dstStages = 0;
	dstAccesses = 0;

	if(!!(after & AccelerationStructureUsageBit::kBuild))
	{
		dstStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		dstAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	}

	if(!!(after & AccelerationStructureUsageBit::kAttach))
	{
		dstStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		dstAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(!!(after & AccelerationStructureUsageBit::kGeometryRead))
	{
		dstStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		dstAccesses |= VK_ACCESS_MEMORY_READ_BIT; // READ_BIT is the only viable solution by elimination
	}

	if(!!(after & AccelerationStructureUsageBit::kFragmentRead))
	{
		dstStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dstAccesses |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(after & AccelerationStructureUsageBit::kComputeRead))
	{
		dstStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstAccesses |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(after & AccelerationStructureUsageBit::kTraceRaysRead))
	{
		dstStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		dstAccesses |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	ANKI_ASSERT(srcStages && dstStages);
}

} // end namespace anki
