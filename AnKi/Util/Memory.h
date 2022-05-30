// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/Assert.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/StackAllocatorBuilder.h>
#include <utility> // For forward

namespace anki {

/// @addtogroup util_memory
/// @{

#define ANKI_MEM_EXTRA_CHECKS ANKI_EXTRA_CHECKS

/// Allocate aligned memory
void* mallocAligned(PtrSize size, PtrSize alignmentBytes);

/// Free aligned memory
void freeAligned(void* ptr);

/// The function signature of a memory allocation/deallocation. See allocAligned function for the explanation of
/// arguments
using AllocAlignedCallback = void* (*)(void* userData, void* ptr, PtrSize size, PtrSize alignment);

/// An internal type.
using PoolSignature = U32;

/// This is a function that allocates and deallocates heap memory. If the @a ptr is nullptr then it allocates using the
/// @a size and @a alignment. If the @a ptr is not nullptr it deallocates the memory and the @a size and @a alignment is
/// ignored.
///
/// @param userData Used defined data
/// @param ptr The pointer to deallocate or nullptr
/// @param size The size to allocate or 0
/// @param alignment The allocation alignment or 0
/// @return On allocation mode it will return the newelly allocated block or nullptr on error. On deallocation mode
///         returns nullptr
void* allocAligned(void* userData, void* ptr, PtrSize size, PtrSize alignment);

/// Generic memory pool. The base of HeapMemoryPool or StackMemoryPool or ChainMemoryPool.
class BaseMemoryPool
{
public:
	BaseMemoryPool(const BaseMemoryPool&) = delete; // Non-copyable

	virtual ~BaseMemoryPool();

	BaseMemoryPool& operator=(const BaseMemoryPool&) = delete; // Non-copyable

	/// Allocate memory. This operation MAY be thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes);

	/// Free memory.
	/// @param[in, out] ptr Memory block to deallocate
	void free(void* ptr);

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	/// Get number of users.
	U32 getUsersCount() const
	{
		return m_refcount.load();
	}

	/// Get allocation callback.
	AllocAlignedCallback getAllocationCallback() const
	{
		return m_allocCb;
	}

	/// Get allocation callback user data.
	void* getAllocationCallbackUserData() const
	{
		return m_allocCbUserData;
	}

	/// Return number of allocations
	U32 getAllocationCount() const
	{
		return m_allocationCount.load();
	}

	/// Get the name of the pool.
	const char* getName() const
	{
		return (m_name) ? m_name : "Unamed";
	}

protected:
	/// Pool type.
	enum class Type : U8
	{
		NONE,
		HEAP,
		STACK,
		CHAIN
	};

	/// User allocation function.
	AllocAlignedCallback m_allocCb = nullptr;

	/// User allocation function data.
	void* m_allocCbUserData = nullptr;

	/// Allocations count.
	Atomic<U32> m_allocationCount = {0};

	/// Construct it.
	BaseMemoryPool(Type type, AllocAlignedCallback allocCb, void* allocCbUserData, const char* name);

private:
	/// Refcount.
	mutable Atomic<I32> m_refcount = {0};

	/// Optional name.
	char* m_name = nullptr;

	/// Type.
	Type m_type = Type::NONE;
};

/// A dummy interface to match the StackMemoryPool and ChainMemoryPool interfaces in order to be used by the same
/// allocator template.
class HeapMemoryPool final : public BaseMemoryPool
{
public:
	/// Construct it.
	/// @param allocCb The allocation function callback.
	/// @param allocCbUserData The user data to pass to the allocation function.
	/// @param name An optional name.
	HeapMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserDataconst, const char* name = nullptr);

	/// Destroy
	~HeapMemoryPool();

	/// The real constructor.
	/// @param allocCb The allocation function callback
	/// @param allocCbUserData The user data to pass to the allocation function
	void init(AllocAlignedCallback allocCb, void* allocCbUserData);

	/// Allocate memory
	void* allocate(PtrSize size, PtrSize alignment);

	/// Free memory.
	/// @param[in, out] ptr Memory block to deallocate.
	void free(void* ptr);

private:
#if ANKI_MEM_EXTRA_CHECKS
	PoolSignature m_signature = 0;
#endif
};

/// Thread safe memory pool. It's a preallocated memory pool that is used for memory allocations on top of that
/// preallocated memory. It is mainly used by fast stack allocators
class StackMemoryPool final : public BaseMemoryPool
{
public:
	/// Init with parameters.
	/// @param allocCb The allocation function callback.
	/// @param allocCbUserData The user data to pass to the allocation function.
	/// @param initialChunkSize The size of the first chunk.
	/// @param nextChunkScale Value that controls the next chunk.
	/// @param nextChunkBias Value that controls the next chunk.
	/// @param ignoreDeallocationErrors Method free() may fail if the ptr is not in the top of the stack. Set that to
	///        true to suppress such errors.
	/// @param alignmentBytes The maximum supported alignment for returned memory.
	/// @param name An optional name.
	StackMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData, PtrSize initialChunkSize,
					F64 nextChunkScale = 2.0, PtrSize nextChunkBias = 0, Bool ignoreDeallocationErrors = true,
					U32 alignmentBytes = ANKI_SAFE_ALIGNMENT, const char* name = nullptr);

	/// Destroy
	~StackMemoryPool();

	/// Allocate aligned memory.
	/// @param size The size to allocate.
	/// @param alignmentBytes The alignment of the returned address.
	/// @return The allocated memory or nullptr on failure.
	///
	/// @note The operation is thread safe with allocate() and free() methods.
	void* allocate(PtrSize size, PtrSize alignmentBytes);

	/// Free memory in StackMemoryPool. It will not actually do anything.
	/// @param[in, out] ptr Memory block to deallocate.
	void free(void* ptr);

	/// Reinit the pool. All existing allocated memory is effectively invalidated.
	/// @note It's not thread safe with other methods.
	void reset();

	/// Get the physical memory allocated by the pool.
	/// @note It's not thread safe with other methods.
	PtrSize getMemoryCapacity() const
	{
		return m_builder.getMemoryCapacity();
	}

private:
	/// This is the absolute max alignment.
	static constexpr U32 MAX_ALIGNMENT = ANKI_SAFE_ALIGNMENT;

	/// This is the chunk the StackAllocatorBuilder will be allocating.
	class alignas(MAX_ALIGNMENT) Chunk
	{
	public:
		/// Required by StackAllocatorBuilder.
		Chunk* m_nextChunk;

		/// Required by StackAllocatorBuilder.
		Atomic<PtrSize> m_offsetInChunk;

		/// Required by StackAllocatorBuilder.
		PtrSize m_chunkSize;

		/// The start of the actual CPU memory.
		alignas(MAX_ALIGNMENT) U8 m_memoryStart[1];
	};

	/// Implements the StackAllocatorBuilder TInterface
	class StackAllocatorBuilderInterface
	{
	public:
		StackMemoryPool* m_parent = nullptr;

		PtrSize m_alignmentBytes = 0;

		Bool m_ignoreDeallocationErrors = false;

		PtrSize m_initialChunkSize = 0;

		F64 m_nextChunkScale = 0.0;

		PtrSize m_nextChunkBias = 0;

		// The rest of the functions implement the StackAllocatorBuilder TInterface.

		PtrSize getMaxAlignment() const
		{
			ANKI_ASSERT(m_alignmentBytes > 0);
			return m_alignmentBytes;
		}

		PtrSize getInitialChunkSize() const
		{
			ANKI_ASSERT(m_initialChunkSize > 0);
			return m_initialChunkSize;
		}

		F64 getNextChunkGrowScale() const
		{
			ANKI_ASSERT(m_nextChunkScale >= 1.0);
			return m_nextChunkScale;
		}

		PtrSize getNextChunkGrowBias() const
		{
			return m_nextChunkBias;
		}

		Bool ignoreDeallocationErrors() const
		{
			return m_ignoreDeallocationErrors;
		}

		Error allocateChunk(PtrSize size, Chunk*& out);

		void freeChunk(Chunk* chunk);

		void recycleChunk(Chunk& chunk);

		Atomic<U32>* getAllocationCount()
		{
			return &m_parent->m_allocationCount;
		}
	};

	/// The allocator helper.
	StackAllocatorBuilder<Chunk, StackAllocatorBuilderInterface, Mutex> m_builder;
};

/// Chain memory pool. Almost similar to StackMemoryPool but more flexible and at the same time a bit slower.
class ChainMemoryPool final : public BaseMemoryPool
{
public:
	/// Init the pool.
	/// @param allocCb The allocation function callback.
	/// @param allocCbUserData The user data to pass to the allocation function.
	/// @param initialChunkSize The size of the first chunk.
	/// @param nextChunkScale Value that controls the next chunk.
	/// @param nextChunkBias Value that controls the next chunk.
	/// @param alignmentBytes The maximum supported alignment for returned memory.
	/// @param name An optional name.
	ChainMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData, PtrSize initialChunkSize,
					F32 nextChunkScale = 2.0, PtrSize nextChunkBias = 0, PtrSize alignmentBytes = ANKI_SAFE_ALIGNMENT,
					const char* name = nullptr);

	/// Destroy
	~ChainMemoryPool();

	/// Allocate memory. This operation is thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes);

	/// Free memory. If the ptr is not the last allocation of the chunk then nothing happens and the method returns
	/// false.
	/// @param[in, out] ptr Memory block to deallocate
	void free(void* ptr);

	/// @name Methods used for optimizing future chains.
	/// @{
	PtrSize getChunksCount() const;

	PtrSize getAllocatedSize() const;
	/// @}

private:
	/// A chunk of memory
	class Chunk
	{
	public:
		/// Pre-allocated memory chunk.
		U8* m_memory = nullptr;

		/// Size of the pre-allocated memory chunk
		PtrSize m_memsize = 0;

		/// Points to the memory and more specifically to the top of the stack
		U8* m_top = nullptr;

		/// Used to identify if the chunk can be deleted
		PtrSize m_allocationCount = 0;

		/// Previous chunk in the list
		Chunk* m_prev = nullptr;

		/// Next chunk in the list
		Chunk* m_next = nullptr;
	};

	/// Alignment of allocations.
	PtrSize m_alignmentBytes = 0;

	/// The first chunk.
	Chunk* m_headChunk = nullptr;

	/// Current chunk to allocate from.
	Chunk* m_tailChunk = nullptr;

	/// Size of the first chunk.
	PtrSize m_initSize = 0;

	/// Fast thread locking.
	SpinLock m_lock;

	/// Chunk scale.
	F32 m_scale = 2.0;

	/// Chunk bias.
	PtrSize m_bias = 0;

	/// Cache a value.
	PtrSize m_headerSize = 0;

	/// Compute the size for the next chunk.
	/// @param size The current allocation size.
	PtrSize computeNewChunkSize(PtrSize size) const;

	/// Create a new chunk.
	Chunk* createNewChunk(PtrSize size);

	/// Allocate from chunk.
	void* allocateFromChunk(Chunk* ch, PtrSize size, PtrSize alignment);

	/// Destroy a chunk.
	void destroyChunk(Chunk* ch);
};

inline void* BaseMemoryPool::allocate(PtrSize size, PtrSize alignmentBytes)
{
	void* out = nullptr;
	switch(m_type)
	{
	case Type::HEAP:
		out = static_cast<HeapMemoryPool*>(this)->allocate(size, alignmentBytes);
		break;
	case Type::STACK:
		out = static_cast<StackMemoryPool*>(this)->allocate(size, alignmentBytes);
		break;
	default:
		ANKI_ASSERT(m_type == Type::CHAIN);
		out = static_cast<ChainMemoryPool*>(this)->allocate(size, alignmentBytes);
	}

	return out;
}

inline void BaseMemoryPool::free(void* ptr)
{
	switch(m_type)
	{
	case Type::HEAP:
		static_cast<HeapMemoryPool*>(this)->free(ptr);
		break;
	case Type::STACK:
		static_cast<StackMemoryPool*>(this)->free(ptr);
		break;
	default:
		ANKI_ASSERT(m_type == Type::CHAIN);
		static_cast<ChainMemoryPool*>(this)->free(ptr);
	}
}
/// @}

} // end namespace anki
