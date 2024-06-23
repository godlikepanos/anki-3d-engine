// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DBuffer.h>
#include <AnKi/Gr/D3D/D3DFrameGarbageCollector.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>

namespace anki {

Buffer* Buffer::newInstance(const BufferInitInfo& init)
{
	BufferImpl* impl = anki::newInstance<BufferImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

void* Buffer::map(PtrSize offset, PtrSize range, BufferMapAccessBit access)
{
	ANKI_D3D_SELF(BufferImpl);

	ANKI_ASSERT(access != BufferMapAccessBit::kNone);
	ANKI_ASSERT((access & m_access) != BufferMapAccessBit::kNone);
	ANKI_ASSERT(!self.m_mapped);
	ANKI_ASSERT(offset < m_size);
	if(range == kMaxPtrSize)
	{
		range = m_size - offset;
	}
	ANKI_ASSERT(offset + range <= m_size);

	D3D12_RANGE d3dRange;
	d3dRange.Begin = offset;
	d3dRange.End = offset + range;

	void* mem = nullptr;
	ANKI_D3D_CHECKF(self.m_resource->Map(0, &d3dRange, &mem));
	ANKI_ASSERT(mem);

#if ANKI_ASSERTIONS_ENABLED
	self.m_mapped = true;
#endif

	return mem;
}

void Buffer::unmap()
{
	ANKI_D3D_SELF(BufferImpl);

	ANKI_ASSERT(self.m_mapped);

#if ANKI_ASSERTIONS_ENABLED
	self.m_mapped = false;
#endif
}

void Buffer::flush([[maybe_unused]] PtrSize offset, [[maybe_unused]] PtrSize range) const
{
	// No-op
}

void Buffer::invalidate([[maybe_unused]] PtrSize offset, [[maybe_unused]] PtrSize range) const
{
	// No-op
}

BufferImpl::~BufferImpl()
{
	ANKI_ASSERT(!m_mapped);

	BufferGarbage* garbage = anki::newInstance<BufferGarbage>(GrMemoryPool::getSingleton());
	garbage->m_resource = m_resource;
	FrameGarbageCollector::getSingleton().newBufferGarbage(garbage);
}

Error BufferImpl::init(const BufferInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());
	m_access = inf.m_mapAccess;
	m_usage = inf.m_usage;
	m_size = inf.m_size;

	// D3D12_HEAP_PROPERTIES
	D3D12_HEAP_PROPERTIES heapProperties = {};
	if(m_access == BufferMapAccessBit::kWrite && !!(m_usage & ~BufferUsageBit::kAllTransfer))
	{
		// Not only transfer, choose ReBAR

		heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;

		if(getGrManagerImpl().getD3DCapabilities().m_rebar)
		{
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
		}
		else
		{
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		}
	}
	else if(m_access == BufferMapAccessBit::kWrite)
	{
		// Only transfer, choose system RAM

		heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	}
	else if(!!(m_access & BufferMapAccessBit::kReadWrite))
	{
		// Reaback, use system RAM

		heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	}
	else
	{
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	}

	// D3D12_HEAP_FLAGS
	D3D12_HEAP_FLAGS heapFlags = {};
	if(!!(m_usage & BufferUsageBit::kAllStorage))
	{
		heapFlags |= D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS;
	}

	// D3D12_RESOURCE_DESC
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	resourceDesc.Width = getAlignedRoundUp(m_size, 256) + 256; // Align to 256 and add padding because of CBV requirements
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = {};
	if(!!(m_usage & (BufferUsageBit::kAllStorage | BufferUsageBit::kAllTexel)))
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	if(!(m_usage & (BufferUsageBit::kAllStorage | BufferUsageBit::kAllUniform | BufferUsageBit::kAllTexel)))
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}

	// Create resource
	const D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
	ANKI_D3D_CHECK(getDevice().CreateCommittedResource(&heapProperties, heapFlags, &resourceDesc, initialState, nullptr, IID_PPV_ARGS(&m_resource)));

	GrDynamicArray<WChar> wstr;
	wstr.resize(getName().getLength() + 1);
	getName().toWideChars(wstr.getBegin(), wstr.getSize());
	ANKI_D3D_CHECK(m_resource->SetName(wstr.getBegin()));

	m_gpuAddress = m_resource->GetGPUVirtualAddress();

	return Error::kNone;
}

D3D12_BUFFER_BARRIER BufferImpl::computeBarrier(BufferUsageBit before, BufferUsageBit after) const
{
	ANKI_ASSERT((m_usage & before) == before);
	ANKI_ASSERT((m_usage & after) == after);

	const D3D12_BUFFER_BARRIER out = {.SyncBefore = computeSync(before),
									  .SyncAfter = computeSync(after),
									  .AccessBefore = computeAccess(before),
									  .AccessAfter = computeAccess(after),
									  .pResource = m_resource,
									  .Offset = 0,
									  .Size = kMaxU64};

	return out;
}

D3D12_BARRIER_SYNC BufferImpl::computeSync(BufferUsageBit usage) const
{
	if(usage == BufferUsageBit::kNone)
	{
		return D3D12_BARRIER_SYNC_NONE;
	}

	const Bool rt = getGrManagerImpl().getDeviceCapabilities().m_rayTracingEnabled;
	D3D12_BARRIER_SYNC sync = {};

	if(!!(usage & BufferUsageBit::kAllIndirect))
	{
		sync |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
	}

	if(!!(usage & BufferUsageBit::kIndex))
	{
		sync |= D3D12_BARRIER_SYNC_INDEX_INPUT;
	}

	if(!!(usage & BufferUsageBit::kAllGeometry))
	{
		sync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
	}

	if(!!(usage & BufferUsageBit::kAllFragment))
	{
		sync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
	}

	if(!!(usage & (BufferUsageBit::kAllCompute & ~BufferUsageBit::kIndirectCompute)))
	{
		sync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
	}

	if(!!(usage & (BufferUsageBit::kAccelerationStructureBuild | BufferUsageBit::kAccelerationStructureBuildScratch)) && rt)
	{
		sync |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
	}

	if(!!(usage & (BufferUsageBit::kAllTraceRays & ~BufferUsageBit::kIndirectTraceRays)) && rt)
	{
		sync |= D3D12_BARRIER_SYNC_RAYTRACING;
	}

	if(!!(usage & BufferUsageBit::kAllTransfer))
	{
		sync |= D3D12_BARRIER_SYNC_COPY;
	}

	ANKI_ASSERT(sync);
	return sync;
}

D3D12_BARRIER_ACCESS BufferImpl::computeAccess(BufferUsageBit usage) const
{
	if(usage == BufferUsageBit::kNone)
	{
		return D3D12_BARRIER_ACCESS_NO_ACCESS;
	}

	D3D12_BARRIER_ACCESS out = {};

	if(!!(usage & BufferUsageBit::kVertex))
	{
		out |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
	}

	if(!!(usage & BufferUsageBit::kAllUniform))
	{
		out |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
	}

	if(!!(usage & BufferUsageBit::kIndex))
	{
		out |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
	}

	if(!!(usage & ((BufferUsageBit::kAllStorage | BufferUsageBit::kAllTexel) & BufferUsageBit::kAllWrite)))
	{
		out |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
	}

	if(!!(usage & ((BufferUsageBit::kAllStorage | BufferUsageBit::kAllTexel) & BufferUsageBit::kAllRead)))
	{
		out |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
	}

	if(!!(usage & BufferUsageBit::kAllIndirect))
	{
		out |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
	}

	if(!!(usage & BufferUsageBit::kTransferDestination))
	{
		out |= D3D12_BARRIER_ACCESS_COPY_DEST;
	}

	if(!!(usage & BufferUsageBit::kTransferSource))
	{
		out |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
	}

	if(!!(usage & (BufferUsageBit::kAccelerationStructureBuild | BufferUsageBit::kAccelerationStructureBuildScratch)))
	{
		out |= D3D12_BARRIER_ACCESS_COMMON;
	}

	if(!!(usage & BufferUsageBit::kShaderBindingTable))
	{
		out |= D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
	}

	ANKI_ASSERT(out);
	return out;
}

} // end namespace anki
