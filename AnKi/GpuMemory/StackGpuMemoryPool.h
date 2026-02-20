// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/GpuMemory/Common.h>
#include <AnKi/Util/StackAllocatorBuilder.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

class StackGpuMemoryPoolAllocation
{
	friend class StackGpuMemoryPool;

public:
	StackGpuMemoryPoolAllocation() = default;

	StackGpuMemoryPoolAllocation(const StackGpuMemoryPoolAllocation&) = delete;

	StackGpuMemoryPoolAllocation(StackGpuMemoryPoolAllocation&& b)
	{
		*this = std::move(b);
	}

	~StackGpuMemoryPoolAllocation() = default;

	StackGpuMemoryPoolAllocation& operator=(const StackGpuMemoryPoolAllocation&) = delete;

	StackGpuMemoryPoolAllocation& operator=(StackGpuMemoryPoolAllocation&& b)
	{
		ANKI_ASSERT(!(*this) && "Need to deallocate first");
		m_buffer = b.m_buffer;
		m_offset = b.m_offset;
		m_size = b.m_size;
		b.reset();
		return *this;
	}

	explicit operator Bool() const
	{
		return m_size != 0;
	}

	operator BufferView() const
	{
		ANKI_ASSERT(!!(*this));
		return BufferView(m_buffer, m_offset, m_size);
	}

	PtrSize getOffset() const
	{
		ANKI_ASSERT(!!(*this));
		return m_offset;
	}

	PtrSize getSize() const
	{
		ANKI_ASSERT(!!(*this));
		return m_size;
	}

	void* getMappedMemory() const
	{
		ANKI_ASSERT(!!(*this));
		ANKI_ASSERT(m_mappedMemory);
		return m_mappedMemory;
	}

private:
	Buffer* m_buffer = nullptr;
	PtrSize m_offset = kMaxPtrSize;
	PtrSize m_size = 0;
	void* m_mappedMemory = nullptr;

	void reset()
	{
		m_buffer = nullptr;
		m_offset = kMaxPtrSize;
		m_size = 0;
		m_mappedMemory = nullptr;
	}
};

// Stack memory pool for GPU usage.
class StackGpuMemoryPool
{
public:
	StackGpuMemoryPool() = default;

	StackGpuMemoryPool(const StackGpuMemoryPool&) = delete; // Non-copyable

	~StackGpuMemoryPool();

	StackGpuMemoryPool& operator=(const StackGpuMemoryPool&) = delete; // Non-copyable

	void init(PtrSize initialSize, F64 nextChunkGrowScale, PtrSize nextChunkGrowBias, BufferUsageBit bufferUsage, BufferMapAccessBit bufferMapping,
			  Bool allowToGrow, CString bufferName);

	// It's thread-safe against other allocate()
	StackGpuMemoryPoolAllocation allocate(PtrSize size, PtrSize alignment);

	// It's thread-safe against other allocate()
	template<typename T>
	StackGpuMemoryPoolAllocation allocateStructuredBuffer(U32 count)
	{
		const U32 alignment = (m_structuredBufferBindOffsetAlignment == kMaxU32) ? sizeof(T) : m_structuredBufferBindOffsetAlignment;
		return allocate(count * sizeof(T), alignment);
	}

	// It's thread-safe against other allocate()
	StackGpuMemoryPoolAllocation allocateStructuredBuffer(U32 count, U32 structureSize)
	{
		const U32 alignment = (m_structuredBufferBindOffsetAlignment == kMaxU32) ? structureSize : m_structuredBufferBindOffsetAlignment;
		return allocate(count * structureSize, alignment);
	}

	void reset();

	PtrSize getAllocatedMemory() const;

private:
	class Chunk;
	class BuilderInterface;
	using Builder = StackAllocatorBuilder<Chunk, BuilderInterface, Mutex>;

	Builder* m_builder = nullptr;

	U32 m_structuredBufferBindOffsetAlignment = kMaxU32;
};

} // end namespace anki
