// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DBuffer.h>
#include <AnKi/Gr/D3D/D3DFrameGarbageCollector.h>

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
	if(m_access == BufferMapAccessBit::kWrite)
	{
		heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	}
	else if(!!(m_access & BufferMapAccessBit::kReadWrite))
	{
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
	resourceDesc.Width = m_size;
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

	// D3D12_RESOURCE_STATES (No need for setting D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	D3D12_RESOURCE_STATES states = {};
	auto appendState = [&](BufferUsageBit usage, D3D12_RESOURCE_STATES addStates) {
		if(!!(m_usage & usage))
		{
			states |= addStates;
		}
	};
	appendState(BufferUsageBit::kAllUniform | BufferUsageBit::kAllGeometry, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	appendState(BufferUsageBit::kIndex, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	appendState(BufferUsageBit::kAllGeometry, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	appendState(BufferUsageBit::kAllFragment, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	appendState(BufferUsageBit::kAllIndirect, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	appendState(BufferUsageBit::kTransferDestination, D3D12_RESOURCE_STATE_COPY_DEST);
	appendState(BufferUsageBit::kTransferSource, D3D12_RESOURCE_STATE_COPY_SOURCE);

	// Create resource
	ANKI_D3D_CHECK(getDevice().CreateCommittedResource(&heapProperties, heapFlags, &resourceDesc, states, nullptr, IID_PPV_ARGS(&m_resource)));

	return Error::kNone;
}

} // end namespace anki
