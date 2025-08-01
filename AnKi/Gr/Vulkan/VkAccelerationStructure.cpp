// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkAccelerationStructure.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Gr/Vulkan/VkBuffer.h>

namespace anki {

AccelerationStructure* AccelerationStructure::newInstance(const AccelerationStructureInitInfo& init)
{
	AccelerationStructureImpl* impl = anki::newInstance<AccelerationStructureImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

U64 AccelerationStructure::getGpuAddress() const
{
	ANKI_VK_SELF_CONST(AccelerationStructureImpl);
	return self.m_deviceAddress;
}

AccelerationStructureImpl::~AccelerationStructureImpl()
{
	if(m_handle)
	{
		vkDestroyAccelerationStructureKHR(getVkDevice(), m_handle, nullptr);
	}
}

Error AccelerationStructureImpl::init(const AccelerationStructureInitInfo& inf)
{
	ANKI_TRACE_SCOPED_EVENT(VkInitAccStruct);

	ANKI_ASSERT(inf.isValid());
	m_type = inf.m_type;
	const VkDevice vkdev = getGrManagerImpl().getDevice();

	PtrSize asBufferSize;
	getMemoryRequirement(inf, asBufferSize, m_scratchBufferSize);
	m_scratchBufferSize += getGrManagerImpl().getVulkanCapabilities().m_asBuildScratchAlignment;

	// Allocate AS buffer
	BufferView asBuff = inf.m_accelerationStructureBuffer;
	if(!asBuff.isValid())
	{
		BufferInitInfo bufferInit(inf.getName());
		bufferInit.m_usage = BufferUsageBit::kAccelerationStructure;
		bufferInit.m_size = asBufferSize;
		m_asBuffer = getGrManagerImpl().newBuffer(bufferInit);
		m_asBufferOffset = 0;
	}
	else
	{
		const PtrSize alignedOffset = getAlignedRoundUp(getGrManagerImpl().getVulkanCapabilities().m_asBufferAlignment, asBuff.getOffset());
		asBuff = asBuff.incrementOffset(alignedOffset - asBuff.getOffset());
		ANKI_ASSERT(asBuff.getRange() <= asBufferSize);

		m_asBuffer.reset(&asBuff.getBuffer());
		m_asBufferOffset = asBuff.getOffset();
	}

	// Create the AS
	if(m_type == AccelerationStructureType::kBottomLevel)
	{
		// Geom
		VkAccelerationStructureGeometryKHR& geom = m_geometry;
		geom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geom.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		geom.geometry.triangles.vertexFormat = convertFormat(inf.m_bottomLevel.m_positionsFormat);
		geom.geometry.triangles.vertexData.deviceAddress =
			inf.m_bottomLevel.m_positionBuffer.getBuffer().getGpuAddress() + inf.m_bottomLevel.m_positionBuffer.getOffset();
		geom.geometry.triangles.vertexStride = inf.m_bottomLevel.m_positionStride;
		geom.geometry.triangles.maxVertex = inf.m_bottomLevel.m_positionCount - 1;
		geom.geometry.triangles.indexType = convertIndexType(inf.m_bottomLevel.m_indexType);
		geom.geometry.triangles.indexData.deviceAddress =
			inf.m_bottomLevel.m_indexBuffer.getBuffer().getGpuAddress() + inf.m_bottomLevel.m_indexBuffer.getOffset();
		geom.flags = 0;

		// Geom build info
		VkAccelerationStructureBuildGeometryInfoKHR& buildInfo = m_buildInfo;
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geom;

		// Create the AS
		VkAccelerationStructureCreateInfoKHR asCi = {};
		asCi.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCi.createFlags = 0;
		asCi.buffer = static_cast<const BufferImpl&>(*m_asBuffer).getHandle();
		asCi.offset = m_asBufferOffset;
		asCi.size = asBufferSize;
		asCi.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		ANKI_VK_CHECK(vkCreateAccelerationStructureKHR(vkdev, &asCi, nullptr, &m_handle));

		// Get its address
		VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {};
		addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		addressInfo.accelerationStructure = m_handle;
		m_deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(vkdev, &addressInfo);

		// Almost finalize the build info
		buildInfo.dstAccelerationStructure = m_handle;

		// Range info
		m_rangeInfo.primitiveCount = inf.m_bottomLevel.m_indexCount / 3;

		m_blas.m_positionsBuffer.reset(&inf.m_bottomLevel.m_positionBuffer.getBuffer());
		m_blas.m_indexBuffer.reset(&inf.m_bottomLevel.m_positionBuffer.getBuffer());
	}
	else
	{
		ANKI_ASSERT(sizeof(VkAccelerationStructureInstanceKHR) * inf.m_topLevel.m_instanceCount <= inf.m_topLevel.m_instancesBuffer.getRange());
		m_tlas.m_instancesBuffer.reset(&inf.m_topLevel.m_instancesBuffer.getBuffer());

		// Geom
		VkAccelerationStructureGeometryKHR& geom = m_geometry;
		geom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geom.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geom.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		geom.geometry.instances.data.deviceAddress = m_tlas.m_instancesBuffer->getGpuAddress() + inf.m_topLevel.m_instancesBuffer.getOffset();
		geom.geometry.instances.arrayOfPointers = false;
		geom.flags = 0;

		// Geom build info
		VkAccelerationStructureBuildGeometryInfoKHR& buildInfo = m_buildInfo;
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geom;

		// Create the AS
		VkAccelerationStructureCreateInfoKHR asCi = {};
		asCi.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCi.createFlags = 0;
		asCi.buffer = static_cast<const BufferImpl&>(*m_asBuffer).getHandle();
		asCi.offset = m_asBufferOffset;
		asCi.size = asBufferSize;
		asCi.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		ANKI_VK_CHECK(vkCreateAccelerationStructureKHR(vkdev, &asCi, nullptr, &m_handle));

		// Almost finalize the build info
		buildInfo.dstAccelerationStructure = m_handle;

		// Range info
		m_rangeInfo.primitiveCount = inf.m_topLevel.m_instanceCount;
	}

	return Error::kNone;
}

VkMemoryBarrier AccelerationStructureImpl::computeBarrierInfo(AccelerationStructureUsageBit before, AccelerationStructureUsageBit after,
															  VkPipelineStageFlags& srcStages_, VkPipelineStageFlags& dstStages_)
{
	VkMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;

	// Before
	VkPipelineStageFlags srcStages = 0;

	if(before == AccelerationStructureUsageBit::kNone)
	{
		srcStages |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		barrier.srcAccessMask |= 0;
	}

	if(!!(before & AccelerationStructureUsageBit::kBuild))
	{
		srcStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		barrier.srcAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	}

	if(!!(before & AccelerationStructureUsageBit::kAttach))
	{
		srcStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		barrier.srcAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(!!(before & AccelerationStructureUsageBit::kSrvGeometry))
	{
		srcStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		barrier.srcAccessMask |= VK_ACCESS_MEMORY_READ_BIT; // READ_BIT is the only viable solution by elimination
	}

	if(!!(before & AccelerationStructureUsageBit::kSrvPixel))
	{
		srcStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.srcAccessMask |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(before & AccelerationStructureUsageBit::kSrvCompute))
	{
		srcStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		barrier.srcAccessMask |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(before & AccelerationStructureUsageBit::kSrvDispatchRays))
	{
		srcStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		barrier.srcAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	// After
	VkPipelineStageFlags dstStages = 0;

	if(!!(after & AccelerationStructureUsageBit::kBuild))
	{
		dstStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		barrier.dstAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	}

	if(!!(after & AccelerationStructureUsageBit::kAttach))
	{
		dstStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		barrier.dstAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(!!(after & AccelerationStructureUsageBit::kSrvGeometry))
	{
		dstStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
					 | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		barrier.dstAccessMask |= VK_ACCESS_MEMORY_READ_BIT; // READ_BIT is the only viable solution by elimination
	}

	if(!!(after & AccelerationStructureUsageBit::kSrvPixel))
	{
		dstStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.dstAccessMask |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(after & AccelerationStructureUsageBit::kSrvCompute))
	{
		dstStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		barrier.dstAccessMask |= VK_ACCESS_MEMORY_READ_BIT;
	}

	if(!!(after & AccelerationStructureUsageBit::kSrvDispatchRays))
	{
		dstStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		barrier.dstAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	ANKI_ASSERT(srcStages && dstStages);

	srcStages_ |= srcStages;
	dstStages_ |= dstStages;

	return barrier;
}

void AccelerationStructureImpl::generateBuildInfo(U64 scratchBufferAddress, VkAccelerationStructureBuildGeometryInfoKHR& buildInfo,
												  VkAccelerationStructureBuildRangeInfoKHR& rangeInfo) const
{
	buildInfo = m_buildInfo;
	buildInfo.scratchData.deviceAddress =
		getAlignedRoundUp(getGrManagerImpl().getVulkanCapabilities().m_asBuildScratchAlignment, scratchBufferAddress);
	rangeInfo = m_rangeInfo;
}

void AccelerationStructureImpl::getMemoryRequirement(const AccelerationStructureInitInfo& inf, PtrSize& asBufferSize, PtrSize& buildScratchBufferSize)
{
	ANKI_ASSERT(inf.isValidForGettingMemoryRequirements());

	VkAccelerationStructureGeometryKHR geom = {};
	geom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;

	VkAccelerationStructureBuildSizesInfoKHR buildSizes = {};
	buildSizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	if(inf.m_type == AccelerationStructureType::kBottomLevel)
	{
		geom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geom.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		geom.geometry.triangles.vertexFormat = convertFormat(inf.m_bottomLevel.m_positionsFormat);
		geom.geometry.triangles.vertexStride = inf.m_bottomLevel.m_positionStride;
		geom.geometry.triangles.maxVertex = inf.m_bottomLevel.m_positionCount - 1;
		geom.geometry.triangles.indexType = convertIndexType(inf.m_bottomLevel.m_indexType);
		geom.flags = 0;

		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geom;

		const U32 primitiveCount = inf.m_bottomLevel.m_indexCount / 3;
		vkGetAccelerationStructureBuildSizesKHR(getVkDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &primitiveCount,
												&buildSizes);
	}
	else
	{
		geom.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geom.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		geom.geometry.instances.arrayOfPointers = false;
		geom.flags = 0;

		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geom;

		vkGetAccelerationStructureBuildSizesKHR(getVkDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
												&inf.m_topLevel.m_instanceCount, &buildSizes);
	}

	asBufferSize = buildSizes.accelerationStructureSize;
	buildScratchBufferSize = buildSizes.buildScratchSize;
}

} // end namespace anki
