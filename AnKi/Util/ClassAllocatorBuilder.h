// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/List.h>
#include <AnKi/Util/DynamicArray.h>

namespace anki {

/// @addtogroup util_memory
/// @{

/// @memberof ClassAllocatorBuilder
class ClassAllocatorBuilderStats
{
public:
	PtrSize m_allocatedSize;
	PtrSize m_inUseSize;
	U32 m_chunkCount; ///< Can be assosiated with the number of allocations.
};

/// This is a convenience class used to build class memory allocators.
/// @tparam TChunk This is the type of the internally allocated chunks. This should be having the following members:
///                @code
///                BitSet<SOME_UPPER_LIMIT> m_inUseSuballocations
///                U32 m_suballocationCount;
///                void* m_class;
///                @endcode
///                And should inherit from IntrusiveListEnabled<TChunk>
/// @tparam TInterface This is the type of the interface that contains various info. Should have the following members:
///                    @code
///                    U32 getClassCount();
///                    void getClassInfo(U32 classIdx, PtrSize& chunkSize, PtrSize& suballocationSize) const;
///                    Error allocateChunk(U32 classIdx, Chunk*& chunk);
///                    void freeChunk(TChunk* out);
///                    @endcode
/// @tparam TLock This an optional lock. Can be a Mutex or SpinLock or some dummy class.
template<typename TChunk, typename TInterface, typename TLock>
class ClassAllocatorBuilder
{
public:
	/// Create.
	ClassAllocatorBuilder() = default;

	ClassAllocatorBuilder(const ClassAllocatorBuilder&) = delete; // Non-copyable

	/// Calls @a destroy().
	~ClassAllocatorBuilder()
	{
		destroy();
	}

	ClassAllocatorBuilder& operator=(const ClassAllocatorBuilder&) = delete; // Non-copyable

	/// Initialize it. Feel free to feedle with the TInterface before you do that.
	void init(BaseMemoryPool* pool);

	/// Destroy the allocator builder.
	void destroy();

	/// Allocate memory.
	/// @param size The size to allocate.
	/// @param alignment The alignment of the returned address.
	/// @param[out] chunk The chunk that the memory belongs to.
	/// @param[out] offset The offset inside the chunk.
	/// @note This is thread safe.
	Error allocate(PtrSize size, PtrSize alignment, TChunk*& chunk, PtrSize& offset);

	/// Free memory.
	/// @param chunk The chunk the allocation belongs to.
	/// @param offset The memory offset inside the chunk.
	void free(TChunk* chunk, PtrSize offset);

	/// Access the interface.
	/// @note Not thread safe. Don't call it while calling allocate or free.
	TInterface& getInterface()
	{
		return m_interface;
	}

	/// Access the interface.
	/// @note Not thread safe. Don't call it while calling allocate or free.
	const TInterface& getInterface() const
	{
		return m_interface;
	}

	/// Get some statistics.
	/// @note It's thread-safe because it will lock. Don't overuse it.
	void getStats(ClassAllocatorBuilderStats& stats) const;

private:
	/// A class of allocations. It's a list of memory chunks. Each chunk is dividied in suballocations.
	class Class
	{
	public:
		/// The active chunks.
		IntrusiveList<TChunk> m_chunkList;

		/// The size of each chunk.
		PtrSize m_chunkSize = 0;

		/// The max size a suballocation can have.
		PtrSize m_suballocationSize = 0;

		/// Lock.
		mutable TLock m_mtx;
	};

	BaseMemoryPool* m_pool = nullptr;

	/// The interface as decribed in the class docs.
	TInterface m_interface;

	/// All the classes.
	DynamicArray<Class> m_classes;

	Class* findClass(PtrSize size, PtrSize alignment);

	Bool isInitialized() const
	{
		return m_classes[0].m_chunkSize != 0;
	}
};
/// @}

} // end namespace anki

#include <AnKi/Util/ClassAllocatorBuilder.inl.h>
