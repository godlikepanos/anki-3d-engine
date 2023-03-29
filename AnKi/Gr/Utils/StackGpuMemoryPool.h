// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Common.h>
#include <AnKi/Util/StackAllocatorBuilder.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Stack memory pool for GPU usage.
class StackGpuMemoryPool
{
public:
	StackGpuMemoryPool() = default;

	StackGpuMemoryPool(const StackGpuMemoryPool&) = delete; // Non-copyable

	~StackGpuMemoryPool();

	StackGpuMemoryPool& operator=(const StackGpuMemoryPool&) = delete; // Non-copyable

	void init(PtrSize initialSize, F64 nextChunkGrowScale, PtrSize nextChunkGrowBias, U32 alignment,
			  BufferUsageBit bufferUsage, BufferMapAccessBit bufferMapping, Bool allowToGrow, CString bufferName);

	/// @note It's thread-safe against other allocate()
	void allocate(PtrSize size, PtrSize& outOffset, Buffer*& buffer)
	{
		void* dummyMapped = nullptr;
		allocate(size, outOffset, buffer, dummyMapped);
	}

	/// @note It's thread-safe against other allocate()
	void allocate(PtrSize size, PtrSize& outOffset, Buffer*& buffer, void*& mappedMemory);

	void reset();

private:
	class Chunk;
	class BuilderInterface;
	using Builder = StackAllocatorBuilder<Chunk, BuilderInterface, Mutex>;

	Builder* m_builder = nullptr;
};
/// @}

} // end namespace anki
