// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DCommon.h>

namespace anki {

/// @addtogroup directx
/// @{

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
		return Error::kNone;
	}

	/// @note Thread-safe.
	D3D12_CPU_DESCRIPTOR_HANDLE allocate()
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
		D3D12_CPU_DESCRIPTOR_HANDLE out;
		out.ptr = m_heapStart.ptr + PtrSize(idx) * m_descriptorSize;
		return out;
	}

	/// @note Thread-safe.
	void free(D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		ANKI_ASSERT(m_heap);
		if(handle.ptr == 0)
		{
			return;
		}

		ANKI_ASSERT(handle.ptr != 0 && handle.ptr >= m_heapStart.ptr
					&& handle.ptr < m_heapStart.ptr + PtrSize(m_descriptorSize) + m_freeDescriptors.getSize());

		const U16 idx = U16((handle.ptr - m_heapStart.ptr) / m_descriptorSize);

		LockGuard lock(m_mtx);
		ANKI_ASSERT(m_freeDescriptorsHead > 0);
		--m_freeDescriptorsHead;
		m_freeDescriptors[m_freeDescriptorsHead] = idx;
	}

private:
	ID3D12DescriptorHeap* m_heap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_heapStart = {};
	U32 m_descriptorSize = 0;

	GrDynamicArray<U16> m_freeDescriptors;
	U16 m_freeDescriptorsHead = 0;

	Mutex m_mtx;
};

class RtvDescriptorHeap : public DescriptorHeap, public MakeSingleton<RtvDescriptorHeap>
{
};
/// @}

} // end namespace anki
