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
	ANKI_ASSERT(!"TODO");
	return 0;
}

AccelerationStructureImpl::~AccelerationStructureImpl()
{
}

Error AccelerationStructureImpl::init(const AccelerationStructureInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	m_type = inf.m_type;

	if(inf.m_type == AccelerationStructureType::kBottomLevel)
	{
		// Setup the geom descr
		m_blas.m_geometryDesc = {};
		m_blas.m_geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		m_blas.m_geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; // TODO
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

		// Get sizes
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		inputs.NumDescs = 1;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.pGeometryDescs = &m_blas.m_geometryDesc;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
		getDevice().GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
		m_scratchBufferSize = prebuildInfo.ScratchDataSizeInBytes;

		// Create the AS buffer
		BufferInitInfo asBuffInit(inf.getName());
		asBuffInit.m_size = prebuildInfo.ResultDataMaxSizeInBytes;
		asBuffInit.m_usage = PrivateBufferUsageBit::kAccelerationStructure;
		m_asBuffer.reset(GrManager::getSingleton().newBuffer(asBuffInit).get());
	}
	else
	{
		const Bool isIndirect = inf.m_topLevel.m_indirectArgs.m_maxInstanceCount > 0;
		const U32 instanceCount = (isIndirect) ? inf.m_topLevel.m_indirectArgs.m_maxInstanceCount : inf.m_topLevel.m_directArgs.m_instances.getSize();

		// Get sizes
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
		inputs.NumDescs = instanceCount;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
		getDevice().GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
		m_scratchBufferSize = prebuildInfo.ScratchDataSizeInBytes;

		// Create the AS buffer
		BufferInitInfo asBuffInit(inf.getName());
		asBuffInit.m_size = prebuildInfo.ResultDataMaxSizeInBytes;
		asBuffInit.m_usage = PrivateBufferUsageBit::kAccelerationStructure;
		m_asBuffer.reset(GrManager::getSingleton().newBuffer(asBuffInit).get());

		// Create instances buffer
		if(!isIndirect)
		{
			BufferInitInfo buffInit("AS instances");
			buffInit.m_size = inf.m_topLevel.m_directArgs.m_instances.getSize() * sizeof(AccelerationStructureInstance);
			buffInit.m_usage = BufferUsageBit::kAllUav;
			buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
			m_tlas.m_instancesBuff.reset(GrManager::getSingleton().newBuffer(buffInit).get());

			WeakArray<AccelerationStructureInstance> mapped(
				static_cast<AccelerationStructureInstance*>(m_tlas.m_instancesBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite)),
				inf.m_topLevel.m_directArgs.m_instances.getSize());

			for(U32 i = 0; i < inf.m_topLevel.m_directArgs.m_instances.getSize(); ++i)
			{
				const AccelerationStructureInstanceInfo& in = inf.m_topLevel.m_directArgs.m_instances[i];
				AccelerationStructureInstance& out = mapped[i];

				const AccelerationStructureImpl& blas = static_cast<const AccelerationStructureImpl&>(*in.m_bottomLevel);
				const U64 blasAddr = blas.m_asBuffer->getGpuAddress();

				const U32 flags =
					D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE | D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;

				out.m_transform = in.m_transform;
				out.m_mask8_instanceCustomIndex24 = (in.m_mask << 24) | (i & 0xFFFFFF);
				out.m_flags8_instanceShaderBindingTableRecordOffset24 = (flags << 24) | in.m_hitgroupSbtRecordIndex;
				memcpy(&out.m_accelerationStructureAddress, &blasAddr, sizeof(blasAddr));
			}

			m_tlas.m_instancesBuff->unmap();
		}
		else
		{
			m_tlas.m_instancesBuff.reset(&inf.m_topLevel.m_indirectArgs.m_instancesBuffer.getBuffer());
			m_tlas.m_instancesBuffOffset = inf.m_topLevel.m_indirectArgs.m_instancesBuffer.getOffset();
		}

		m_tlas.m_instanceCount = instanceCount;
	}

	return Error::kNone;
}

void AccelerationStructureImpl::fillBuildInfo(BufferView scratchBuff, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& buildDesc) const
{
	buildDesc = {};
	buildDesc.DestAccelerationStructureData = m_asBuffer->getGpuAddress();
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

D3D12_GLOBAL_BARRIER AccelerationStructureImpl::computeBarrierInfo(AccelerationStructureUsageBit before, AccelerationStructureUsageBit after) const
{
	D3D12_GLOBAL_BARRIER barrier = {};

	if(before == AccelerationStructureUsageBit::kNone)
	{
		barrier.SyncBefore |= D3D12_BARRIER_SYNC_NONE;
		barrier.AccessBefore |= D3D12_BARRIER_ACCESS_NO_ACCESS;
	}

	if(!!(before & AccelerationStructureUsageBit::kBuild))
	{
		barrier.SyncBefore |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
		barrier.AccessBefore |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
	}

	if(!!(before & AccelerationStructureUsageBit::kAttach))
	{
		barrier.SyncBefore |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
		barrier.AccessBefore |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	if(!!(before & AccelerationStructureUsageBit::kGeometrySrv))
	{
		barrier.SyncBefore |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
		barrier.AccessBefore |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ; // READ_BIT is the only viable solution by elimination
	}

	if(!!(before & AccelerationStructureUsageBit::kPixelSrv))
	{
		barrier.SyncBefore |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
		barrier.AccessBefore |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	if(!!(before & AccelerationStructureUsageBit::kComputeSrv))
	{
		barrier.SyncBefore |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
		barrier.AccessBefore |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	if(!!(before & AccelerationStructureUsageBit::kTraceRaysSrv))
	{
		barrier.SyncBefore |= D3D12_BARRIER_SYNC_RAYTRACING;
		barrier.AccessBefore |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	// After
	if(!!(after & AccelerationStructureUsageBit::kBuild))
	{
		barrier.SyncAfter |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
		barrier.AccessAfter |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
	}

	if(!!(after & AccelerationStructureUsageBit::kAttach))
	{
		barrier.SyncAfter |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
		barrier.AccessAfter |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	if(!!(after & AccelerationStructureUsageBit::kGeometrySrv))
	{
		barrier.SyncAfter |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
		barrier.AccessAfter |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ; // READ_BIT is the only viable solution by elimination
	}

	if(!!(after & AccelerationStructureUsageBit::kPixelSrv))
	{
		barrier.SyncAfter |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
		barrier.AccessAfter |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	if(!!(after & AccelerationStructureUsageBit::kComputeSrv))
	{
		barrier.SyncAfter |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
		barrier.AccessAfter |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	if(!!(after & AccelerationStructureUsageBit::kTraceRaysSrv))
	{
		barrier.SyncAfter |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
		barrier.AccessAfter |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	ANKI_ASSERT(barrier.SyncBefore || barrier.SyncAfter);

	return barrier;
}

} // end namespace anki
