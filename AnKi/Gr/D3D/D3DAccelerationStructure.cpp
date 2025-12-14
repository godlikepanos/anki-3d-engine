// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DAccelerationStructure.h>
#include <AnKi/Gr/D3D/D3DBuffer.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>

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
	ANKI_D3D_SELF_CONST(AccelerationStructureImpl);
	return self.m_asBuffer->getGpuAddress() + self.m_asBufferOffset;
}

AccelerationStructureImpl::~AccelerationStructureImpl()
{
}

Error AccelerationStructureImpl::init(const AccelerationStructureInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	m_type = inf.m_type;

	PtrSize asBufferSize;
	getMemoryRequirement(inf, asBufferSize, m_scratchBufferSize, false);
	m_scratchBufferSize += kScratchBufferAlignment;

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
		const PtrSize alignedOffset = getAlignedRoundUp(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, asBuff.getOffset());
		asBuff = asBuff.incrementOffset(alignedOffset - asBuff.getOffset());
		ANKI_ASSERT(asBuff.getRange() <= asBufferSize);

		m_asBuffer.reset(&asBuff.getBuffer());
		m_asBufferOffset = asBuff.getOffset();
	}

	if(inf.m_type == AccelerationStructureType::kBottomLevel)
	{
		// Setup the geom descr
		m_blas.m_geometryDesc = {};
		m_blas.m_geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		m_blas.m_geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
		m_blas.m_geometryDesc.Triangles.Transform3x4 = 0;
		m_blas.m_geometryDesc.Triangles.IndexFormat = convertIndexType(inf.m_bottomLevel.m_indexType);
		m_blas.m_geometryDesc.Triangles.VertexFormat = convertFormat(inf.m_bottomLevel.m_positionsFormat);
		m_blas.m_geometryDesc.Triangles.IndexCount = inf.m_bottomLevel.m_indexCount;
		m_blas.m_geometryDesc.Triangles.VertexCount = inf.m_bottomLevel.m_positionCount;
		m_blas.m_geometryDesc.Triangles.IndexBuffer =
			inf.m_bottomLevel.m_indexBuffer.getBuffer().getGpuAddress() + inf.m_bottomLevel.m_indexBuffer.getOffset();
		m_blas.m_geometryDesc.Triangles.VertexBuffer.StartAddress =
			inf.m_bottomLevel.m_positionBuffer.getBuffer().getGpuAddress() + inf.m_bottomLevel.m_positionBuffer.getOffset();
		m_blas.m_geometryDesc.Triangles.VertexBuffer.StrideInBytes = inf.m_bottomLevel.m_positionStride;

		m_blas.m_indexBuff.reset(&inf.m_bottomLevel.m_indexBuffer.getBuffer());
		m_blas.m_positionsBuff.reset(&inf.m_bottomLevel.m_positionBuffer.getBuffer());
	}
	else
	{
		m_tlas.m_instancesBuff.reset(&inf.m_topLevel.m_instancesBuffer.getBuffer());
		m_tlas.m_instancesBuffOffset = inf.m_topLevel.m_instancesBuffer.getOffset();
		m_tlas.m_instanceCount = inf.m_topLevel.m_instanceCount;
	}

	return Error::kNone;
}

void AccelerationStructureImpl::fillBuildInfo(BufferView scratchBuff, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& buildDesc) const
{
	ANKI_ASSERT(scratchBuff.getRange() == m_scratchBufferSize);
	buildDesc = {};
	buildDesc.DestAccelerationStructureData = m_asBuffer->getGpuAddress() + m_asBufferOffset;

	const PtrSize offsetDiff = getAlignedRoundUp(kScratchBufferAlignment, scratchBuff.getOffset()) - scratchBuff.getOffset();
	scratchBuff = scratchBuff.incrementOffset(offsetDiff);
	buildDesc.ScratchAccelerationStructureData = scratchBuff.getBuffer().getGpuAddress() + scratchBuff.getOffset();

	if(m_type == AccelerationStructureType::kBottomLevel)
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs = buildDesc.Inputs;
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		inputs.NumDescs = 1;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.pGeometryDescs = &m_blas.m_geometryDesc;
	}
	else
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs = buildDesc.Inputs;
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
		inputs.NumDescs = m_tlas.m_instanceCount;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.InstanceDescs = m_tlas.m_instancesBuff->getGpuAddress() + m_tlas.m_instancesBuffOffset;
	}
}

D3D12_BARRIER_SYNC AccelerationStructureImpl::computeBarrierSync(AccelerationStructureUsageBit usage)
{
	D3D12_BARRIER_SYNC out = {};

	if(usage == AccelerationStructureUsageBit::kNone)
	{
		out |= D3D12_BARRIER_SYNC_NONE;
	}

	if(!!(usage & (AccelerationStructureUsageBit::kBuild | AccelerationStructureUsageBit::kAttach)))
	{
		out |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
	}

	if(!!(usage & (AccelerationStructureUsageBit::kAllSrv)))
	{
		out |= D3D12_BARRIER_SYNC_ALL_SHADING;
	}

	return out;
}

D3D12_BARRIER_ACCESS AccelerationStructureImpl::computeBarrierAccess(AccelerationStructureUsageBit usage)
{
	D3D12_BARRIER_ACCESS out = {};

	if(usage == AccelerationStructureUsageBit::kNone)
	{
		return D3D12_BARRIER_ACCESS_NO_ACCESS;
	}

	if(!!(usage & AccelerationStructureUsageBit::kAllRead))
	{
		out |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	if(!!(usage & AccelerationStructureUsageBit::kAllWrite))
	{
		out |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
	}

	return out;
}

D3D12_GLOBAL_BARRIER AccelerationStructureImpl::computeBarrierInfo(AccelerationStructureUsageBit before, AccelerationStructureUsageBit after) const
{
	const D3D12_GLOBAL_BARRIER barrier = {.SyncBefore = computeBarrierSync(before),
										  .SyncAfter = computeBarrierSync(after),
										  .AccessBefore = computeBarrierAccess(before),
										  .AccessAfter = computeBarrierAccess(after)};

	ANKI_ASSERT(barrier.SyncBefore || barrier.SyncAfter);
	return barrier;
}

void AccelerationStructureImpl::getMemoryRequirement(const AccelerationStructureInitInfo& inf, PtrSize& asBufferSize, PtrSize& buildScratchBufferSize,
													 Bool alignSizes)
{
	ANKI_ASSERT(inf.isValidForGettingMemoryRequirements());

	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};

	if(inf.m_type == AccelerationStructureType::kBottomLevel)
	{
		geomDesc = {};
		geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
		geomDesc.Triangles.Transform3x4 = 0;
		geomDesc.Triangles.IndexFormat = convertIndexType(inf.m_bottomLevel.m_indexType);
		geomDesc.Triangles.VertexFormat = convertFormat(inf.m_bottomLevel.m_positionsFormat);
		geomDesc.Triangles.IndexCount = inf.m_bottomLevel.m_indexCount;
		geomDesc.Triangles.VertexCount = inf.m_bottomLevel.m_positionCount;
		geomDesc.Triangles.VertexBuffer.StrideInBytes = inf.m_bottomLevel.m_positionStride;

		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		inputs.NumDescs = 1;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.pGeometryDescs = &geomDesc;

		getDevice().GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
	}
	else
	{
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
		inputs.NumDescs = inf.m_topLevel.m_instanceCount;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	}

	getDevice().GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
	asBufferSize = prebuildInfo.ResultDataMaxSizeInBytes;
	buildScratchBufferSize = prebuildInfo.ScratchDataSizeInBytes;

	if(alignSizes)
	{
		asBufferSize += D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT;
		buildScratchBufferSize += kScratchBufferAlignment;
	}
}

} // end namespace anki
