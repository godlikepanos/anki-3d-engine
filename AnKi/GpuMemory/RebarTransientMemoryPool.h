// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/GpuMemory/Common.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

ANKI_CVAR(NumericCVar<PtrSize>, GpuMem, RebarGpuMemorySize, 24_MB, 1_MB, 1_GB, "ReBAR: always mapped GPU memory")

// Manages staging GPU memory.
class RebarTransientMemoryPool : public MakeSingleton<RebarTransientMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	RebarTransientMemoryPool(const RebarTransientMemoryPool&) = delete; // Non-copyable

	RebarTransientMemoryPool& operator=(const RebarTransientMemoryPool&) = delete; // Non-copyable

	void init();

	void endFrame(Fence* fence);

	// Allocate staging memory for various operations. The memory will be reused when it's safe
	template<typename T>
	BufferView allocate(PtrSize size, U32 alignment, T*& mappedMem)
	{
		void* mem;
		const BufferView out = allocateInternal(size, alignment, mem);
		mappedMem = static_cast<T*>(mem);
		return out;
	}

	// See allocate()
	template<typename T>
	BufferView allocateConstantBuffer(T*& mappedMem)
	{
		return allocate(sizeof(T), m_constantBufferBindOffsetAlignment, mappedMem);
	}

	// See allocate()
	template<typename T>
	BufferView allocateStructuredBuffer(U32 count, WeakArray<T>& arr)
	{
		T* mem;
		const U32 alignment = (m_structuredBufferAlignment == kMaxU32) ? sizeof(T) : m_structuredBufferAlignment;
		const BufferView out = allocate(count * sizeof(T), alignment, mem);
		arr = {mem, count};
		return out;
	}

	// See allocate()
	template<typename T>
	BufferView allocateCopyBuffer(U32 count, WeakArray<T>& arr)
	{
		T* mem;
		const U32 alignment = sizeof(U32);
		const BufferView out = allocate(sizeof(T) * count, alignment, mem);
		arr = {mem, count};
		return out;
	}

	ANKI_PURE Buffer& getBuffer() const
	{
		return *m_buffer;
	}

	U8* getBufferMappedAddress()
	{
		return m_mappedMem;
	}

private:
	BufferPtr m_buffer;
	U8* m_mappedMem = nullptr; // Cache it
	PtrSize m_bufferSize = 0; // Cache it
	U32 m_structuredBufferAlignment = kMaxU32; // Cache it
	U32 m_constantBufferBindOffsetAlignment = kMaxU32; // Cache it

	Atomic<PtrSize> m_offset = {0};

	// This is the slice of the ReBAR buffer that is protected by a fence
	class FrameSlice
	{
	public:
		PtrSize m_offset = kMaxPtrSize;
		PtrSize m_range = 0;
	};

	static constexpr U32 kSliceCount = 8; // It's actually "max slices in-flight"
	BitSet<kSliceCount, U32> m_activeSliceMask = {false};
	U32 m_crntActiveSlice = kMaxU32;
	Array<FrameSlice, kSliceCount> m_slices;
	Array<FencePtr, kSliceCount> m_sliceFences;

	RebarTransientMemoryPool() = default;

	~RebarTransientMemoryPool();

	BufferView allocateInternal(PtrSize size, U32 alignment, void*& mappedMem);

	void validateSlices() const;
};

} // end namespace anki
