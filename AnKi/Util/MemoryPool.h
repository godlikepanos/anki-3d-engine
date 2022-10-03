// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/Assert.h>
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

/// Generic memory pool. The base of HeapMemoryPool or StackMemoryPool.
class BaseMemoryPool
{
public:
	BaseMemoryPool(const BaseMemoryPool&) = delete; // Non-copyable

	virtual ~BaseMemoryPool()
	{
	}

	BaseMemoryPool& operator=(const BaseMemoryPool&) = delete; // Non-copyable

	/// Allocate memory. This operation MAY be thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes);

	/// Free memory.
	/// @param[in, out] ptr Memory block to deallocate
	void free(void* ptr);

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
	const Char* getName() const
	{
		return (m_name) ? m_name : "Unamed";
	}

protected:
	/// Pool type.
	enum class Type : U8
	{
		kNone,
		kHeap,
		kStack,
	};

	/// User allocation function.
	AllocAlignedCallback m_allocCb = nullptr;

	/// User allocation function data.
	void* m_allocCbUserData = nullptr;

	/// Allocations count.
	Atomic<U32> m_allocationCount = {0};

	BaseMemoryPool(Type type)
		: m_type(type)
	{
	}

	void init(AllocAlignedCallback allocCb, void* allocCbUserData, const Char* name);

	void destroy();

private:
	/// Optional name.
	char* m_name = nullptr;

	/// Type.
	Type m_type = Type::kNone;
};

/// A dummy interface to match the StackMemoryPool interfaces in order to be used by the same allocator template.
class HeapMemoryPool : public BaseMemoryPool
{
public:
	/// Construct it.
	HeapMemoryPool()
		: BaseMemoryPool(Type::kHeap)
	{
	}

	/// @see init
	HeapMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData, const Char* name = nullptr)
		: HeapMemoryPool()
	{
		init(allocCb, allocCbUserData, name);
	}

	/// Destroy
	~HeapMemoryPool()
	{
		destroy();
	}

	/// Init.
	/// @param allocCb The allocation function callback.
	/// @param allocCbUserData The user data to pass to the allocation function.
	/// @param name An optional name.
	void init(AllocAlignedCallback allocCb, void* allocCbUserData, const Char* name = nullptr);

	/// Manual destroy. The destructor calls that as well.
	void destroy();

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
class StackMemoryPool : public BaseMemoryPool
{
public:
	StackMemoryPool()
		: BaseMemoryPool(Type::kStack)
	{
	}

	/// @see init
	StackMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData, PtrSize initialChunkSize,
					F64 nextChunkScale = 2.0, PtrSize nextChunkBias = 0, Bool ignoreDeallocationErrors = true,
					U32 alignmentBytes = ANKI_SAFE_ALIGNMENT, const Char* name = nullptr)
		: StackMemoryPool()
	{
		init(allocCb, allocCbUserData, initialChunkSize, nextChunkScale, nextChunkBias, ignoreDeallocationErrors,
			 alignmentBytes, name);
	}

	/// Destroy
	~StackMemoryPool()
	{
		destroy();
	}

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
	void init(AllocAlignedCallback allocCb, void* allocCbUserData, PtrSize initialChunkSize, F64 nextChunkScale = 2.0,
			  PtrSize nextChunkBias = 0, Bool ignoreDeallocationErrors = true, U32 alignmentBytes = ANKI_SAFE_ALIGNMENT,
			  const Char* name = nullptr);

	/// Manual destroy. The destructor calls that as well.
	void destroy();

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
	static constexpr U32 kMaxAlignment = ANKI_SAFE_ALIGNMENT;

	/// This is the chunk the StackAllocatorBuilder will be allocating.
	class alignas(kMaxAlignment) Chunk
	{
	public:
		/// Required by StackAllocatorBuilder.
		Chunk* m_nextChunk;

		/// Required by StackAllocatorBuilder.
		Atomic<PtrSize> m_offsetInChunk;

		/// Required by StackAllocatorBuilder.
		PtrSize m_chunkSize;

		/// The start of the actual CPU memory.
		alignas(kMaxAlignment) U8 m_memoryStart[1];
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
			return (m_parent) ? &m_parent->m_allocationCount : nullptr;
		}
	};

	/// The allocator helper.
	StackAllocatorBuilder<Chunk, StackAllocatorBuilderInterface, Mutex> m_builder;
};

/// A wrapper class that makes a pointer to a memory pool act like a reference.
template<typename TMemPool>
class MemoryPoolPtrWrapper
{
public:
	TMemPool* m_pool = nullptr;

	MemoryPoolPtrWrapper() = default;

	MemoryPoolPtrWrapper(TMemPool* pool)
		: m_pool(pool)
	{
		ANKI_ASSERT(pool);
	}

	TMemPool* operator&()
	{
		ANKI_ASSERT(m_pool);
		return m_pool;
	}

	operator TMemPool&()
	{
		ANKI_ASSERT(m_pool);
		return *m_pool;
	}

	void* allocate(PtrSize size, PtrSize alignmentBytes)
	{
		ANKI_ASSERT(m_pool);
		return m_pool->allocate(size, alignmentBytes);
	}

	void free(void* ptr)
	{
		ANKI_ASSERT(m_pool);
		m_pool->free(ptr);
	}
};

/// A wrapper class that adds a refcount to a memory pool.
template<typename TMemPool>
class RefCountedMemoryPool : public TMemPool
{
public:
	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

private:
	mutable Atomic<I32> m_refcount = {0};
};

inline void* BaseMemoryPool::allocate(PtrSize size, PtrSize alignmentBytes)
{
	void* out = nullptr;
	switch(m_type)
	{
	case Type::kHeap:
		out = static_cast<HeapMemoryPool*>(this)->allocate(size, alignmentBytes);
		break;
	case Type::kStack:
		out = static_cast<StackMemoryPool*>(this)->allocate(size, alignmentBytes);
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

inline void BaseMemoryPool::free(void* ptr)
{
	switch(m_type)
	{
	case Type::kHeap:
		static_cast<HeapMemoryPool*>(this)->free(ptr);
		break;
	case Type::kStack:
		static_cast<StackMemoryPool*>(this)->free(ptr);
		break;
	default:
		ANKI_ASSERT(0);
	}
}
/// @}

} // end namespace anki
