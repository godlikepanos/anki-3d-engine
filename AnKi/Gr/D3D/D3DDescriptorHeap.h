// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DCommon.h>

namespace anki {

// Forward
class DescriptorHeap;

/// @addtogroup directx
/// @{

/// @memberof DescriptorHeap.
class DescriptorHeapHandle
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {};
	D3D12_DESCRIPTOR_HEAP_TYPE m_heapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;

	[[nodiscard]] Bool isCreated() const
	{
		return m_cpuHandle.ptr != 0;
	}
};

class DescriptorHeap
{
public:
	DescriptorHeap() = default;

	~DescriptorHeap()
	{
		ANKI_ASSERT(m_freeDescriptorsHead == 0 && "Forgot to free");
		safeRelease(m_heap);
	}

	Error init(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, U16 descriptorCount, U32 descriptorSize)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = descriptorCount;
		heapDesc.Type = type;
		heapDesc.Flags = flags;
		ANKI_D3D_CHECK(getDevice().CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
		m_heapStart = m_heap->GetCPUDescriptorHandleForHeapStart();

		m_freeDescriptors.resize(descriptorCount);
		for(U16 i = 0; i < descriptorCount; ++i)
		{
			m_freeDescriptors[i] = i;
		}

		m_freeDescriptorsHead = 0;
		m_descriptorSize = descriptorSize;
		m_type = type;
		return Error::kNone;
	}

	/// @note Thread-safe.
	DescriptorHeapHandle allocate()
	{
		ANKI_ASSERT(m_heap);
		U16 idx;
		{
			LockGuard lock(m_mtx);
			if(m_freeDescriptorsHead == m_freeDescriptors.getSize())
			{
				ANKI_D3D_LOGF("Out of descriptors");
			}
			idx = m_freeDescriptorsHead;
			++m_freeDescriptorsHead;
		}

		idx = m_freeDescriptors[idx];
		DescriptorHeapHandle out;
		out.m_cpuHandle.ptr = m_heapStart.ptr + PtrSize(idx) * m_descriptorSize;
		out.m_heapType = m_type;

		return out;
	}

	/// @note Thread-safe.
	void free(DescriptorHeapHandle& handle)
	{
		ANKI_ASSERT(m_heap);
		if(!handle.isCreated())
		{
			return;
		}

		ANKI_ASSERT(handle.m_heapType == m_type);
		ANKI_ASSERT(handle.m_cpuHandle.ptr != 0 && handle.m_cpuHandle.ptr >= m_heapStart.ptr
					&& handle.m_cpuHandle.ptr < m_heapStart.ptr + m_descriptorSize * m_freeDescriptors.getSize());

		const U16 idx = U16((handle.m_cpuHandle.ptr - m_heapStart.ptr) / m_descriptorSize);

		LockGuard lock(m_mtx);
		ANKI_ASSERT(m_freeDescriptorsHead > 0);
		--m_freeDescriptorsHead;
		m_freeDescriptors[m_freeDescriptorsHead] = idx;
		handle = {};
	}

private:
	ID3D12DescriptorHeap* m_heap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_heapStart = {};
	D3D12_DESCRIPTOR_HEAP_TYPE m_type = {};
	U32 m_descriptorSize = 0;

	GrDynamicArray<U16> m_freeDescriptors;
	U16 m_freeDescriptorsHead = 0;

	Mutex m_mtx;
};

/// An array of all descriptor heaps.
class DescriptorHeaps : public MakeSingleton<DescriptorHeaps>
{
public:
	Error init(const Array<U16, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES>& descriptorCounts, ID3D12Device& dev)
	{
		const Array<D3D12_DESCRIPTOR_HEAP_FLAGS, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> flags = {
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE};

		for(D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
			type = D3D12_DESCRIPTOR_HEAP_TYPE(U32(type) + 1))
		{
			if(descriptorCounts[type])
			{
				ANKI_CHECK(m_heaps[type].init(type, flags[type], descriptorCounts[type], dev.GetDescriptorHandleIncrementSize(type)));
			}
		}

		return Error::kNone;
	}

	/// @note Thread-safe.
	DescriptorHeapHandle allocate(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		return m_heaps[type].allocate();
	}

	/// @note Thread-safe.
	void free(DescriptorHeapHandle& handle)
	{
		if(handle.isCreated())
		{
			m_heaps[handle.m_heapType].free(handle);
		}
	}

private:
	Array<DescriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_heaps;
};
/// @}

} // end namespace anki
