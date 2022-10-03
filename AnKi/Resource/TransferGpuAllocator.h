// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Util/StackAllocatorBuilder.h>
#include <AnKi/Util/List.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

/// @addtogroup resource
/// @{

/// Memory handle.
class TransferGpuAllocatorHandle
{
	friend class TransferGpuAllocator;

public:
	TransferGpuAllocatorHandle()
	{
	}

	TransferGpuAllocatorHandle(const TransferGpuAllocatorHandle&) = delete;

	TransferGpuAllocatorHandle(TransferGpuAllocatorHandle&& b)
	{
		*this = std::move(b);
	}

	~TransferGpuAllocatorHandle()
	{
		ANKI_ASSERT(!valid() && "Forgot to release");
	}

	TransferGpuAllocatorHandle& operator=(TransferGpuAllocatorHandle&& b)
	{
		m_buffer = b.m_buffer;
		m_mappedMemory = b.m_mappedMemory;
		m_offsetInBuffer = b.m_offsetInBuffer;
		m_range = b.m_range;
		m_pool = b.m_pool;
		b.invalidate();
		return *this;
	}

	const BufferPtr& getBuffer() const
	{
		return m_buffer;
	}

	void* getMappedMemory() const
	{
		ANKI_ASSERT(m_mappedMemory);
		return m_mappedMemory;
	}

	PtrSize getOffset() const
	{
		ANKI_ASSERT(m_offsetInBuffer != kMaxPtrSize);
		return m_offsetInBuffer;
	}

	PtrSize getRange() const
	{
		ANKI_ASSERT(m_range != 0);
		return m_range;
	}

private:
	BufferPtr m_buffer;
	void* m_mappedMemory = nullptr;
	PtrSize m_offsetInBuffer = kMaxPtrSize;
	PtrSize m_range = 0;
	U8 m_pool = kMaxU8;

	Bool valid() const
	{
		return m_range != 0 && m_pool < kMaxU8;
	}

	void invalidate()
	{
		m_buffer.reset(nullptr);
		m_mappedMemory = nullptr;
		m_offsetInBuffer = kMaxPtrSize;
		m_range = 0;
		m_pool = kMaxU8;
	}
};

/// GPU memory allocator for GPU buffers used in transfer operations.
class TransferGpuAllocator
{
	friend class TransferGpuAllocatorHandle;

public:
	/// Choose an alignment that satisfies 16 bytes and 3 bytes. RGB8 formats require 3 bytes alignment for the source
	/// of the buffer to image copies.
	static constexpr U32 kGpuBufferAlignment = 16 * 3;

	static constexpr U32 kPoolCount = 2;
	static constexpr PtrSize kChunkInitialSize = 64_MB;
	static constexpr Second kMaxFenceWaitTime = 500.0_ms;

	TransferGpuAllocator();

	~TransferGpuAllocator();

	Error init(PtrSize maxSize, GrManager* gr, HeapMemoryPool* pool);

	/// Allocate some transfer memory. If there is not enough memory it will block until some is releaced. It's
	/// threadsafe.
	Error allocate(PtrSize size, TransferGpuAllocatorHandle& handle);

	/// Release the memory. It will not be recycled before the fence is signaled. It's threadsafe.
	void release(TransferGpuAllocatorHandle& handle, FencePtr fence);

private:
	/// This is the chunk the StackAllocatorBuilder will be allocating.
	class Chunk
	{
	public:
		/// Required by StackAllocatorBuilder.
		Chunk* m_nextChunk;

		/// Required by StackAllocatorBuilder.
		Atomic<PtrSize> m_offsetInChunk;

		/// Required by StackAllocatorBuilder.
		PtrSize m_chunkSize;

		/// The GPU memory.
		BufferPtr m_buffer;

		/// Points to the mapped m_buffer.
		void* m_mappedBuffer;
	};

	/// Implements the StackAllocatorBuilder TInterface
	class StackAllocatorBuilderInterface
	{
	public:
		TransferGpuAllocator* m_parent = nullptr;

		// The rest of the functions implement the StackAllocatorBuilder TInterface.

		constexpr PtrSize getMaxAlignment()
		{
			return kGpuBufferAlignment;
		}

		constexpr PtrSize getInitialChunkSize() const
		{
			return kChunkInitialSize;
		}

		constexpr F64 getNextChunkGrowScale() const
		{
			return 1.0;
		}

		constexpr PtrSize getNextChunkGrowBias() const
		{
			return 0;
		}

		constexpr Bool ignoreDeallocationErrors() const
		{
			return false;
		}

		Error allocateChunk(PtrSize size, Chunk*& out);

		void freeChunk(Chunk* chunk);

		void recycleChunk([[maybe_unused]] Chunk& chunk)
		{
			// Do nothing
		}

		constexpr Atomic<U32>* getAllocationCount()
		{
			return nullptr;
		}
	};

	/// StackAllocatorBuilder doesn't need a lock. There is another lock.
	class DummyMutex
	{
	public:
		void lock()
		{
		}

		void unlock()
		{
		}
	};

	class Pool
	{
	public:
		StackAllocatorBuilder<Chunk, StackAllocatorBuilderInterface, DummyMutex> m_stackAlloc;
		List<FencePtr> m_fences;
		U32 m_pendingReleases = 0;
	};

	HeapMemoryPool* m_pool = nullptr;
	GrManager* m_gr = nullptr;
	PtrSize m_maxAllocSize = 0;

	Mutex m_mtx; ///< Protect all members bellow.
	ConditionVariable m_condVar;
	Array<Pool, kPoolCount> m_pools;
	U8 m_crntPool = 0;
	PtrSize m_crntPoolAllocatedSize = 0;
};
/// @}

} // end namespace anki
