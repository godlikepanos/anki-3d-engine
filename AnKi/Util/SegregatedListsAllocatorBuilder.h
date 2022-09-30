// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Array.h>
#include <AnKi/Util/DynamicArray.h>
#include <AnKi/Util/StringList.h>

namespace anki {

/// @addtogroup util_memory
/// @{

namespace detail {

/// Free block.
/// @memberof SegregatedListsAllocatorBuilder
/// @internal
class SegregatedListsAllocatorBuilderFreeBlock
{
public:
	PtrSize m_size = 0;
	PtrSize m_address = 0;
};

} // end namespace detail

/// The base class for all user memory chunks of SegregatedListsAllocatorBuilder.
/// @memberof SegregatedListsAllocatorBuilder
template<typename TChunk, U32 kClassCount>
class SegregatedListsAllocatorBuilderChunkBase
{
	template<typename TChunk_, typename TInterface, typename TLock>
	friend class SegregatedListsAllocatorBuilder;

private:
	Array<DynamicArray<detail::SegregatedListsAllocatorBuilderFreeBlock>, kClassCount> m_freeLists;
	PtrSize m_totalSize = 0;
	PtrSize m_freeSize = 0;
};

/// It provides the tools to build allocators base on segregated lists and best fit.
/// @tparam TChunk A user defined class that contains a memory chunk. It should inherit from
///                SegregatedListsAllocatorBuilderChunkBase.
/// @tparam TInterface The interface that contains the following members:
///                    @code
///                    /// The number of classes
///                    static constexpr U32 getClassCount();
///                    /// Max size for each class.
///                    void getClassInfo(U32 idx, PtrSize& size) const;
///                    /// Allocates a new user defined chunk of memory.
///                    Error allocateChunk(TChunk*& newChunk, PtrSize& chunkSize);
///                    /// Deletes a chunk.
///                    void deleteChunk(TChunk* chunk);
///                    /// Get an allocator for internal allocations of the builder.
///                    SomeMemoryPool& getMemoryPool() const;
///                    @endcode
/// @tparam TLock User defined lock (eg Mutex).
template<typename TChunk, typename TInterface, typename TLock>
class SegregatedListsAllocatorBuilder
{
public:
	SegregatedListsAllocatorBuilder() = default;

	~SegregatedListsAllocatorBuilder();

	SegregatedListsAllocatorBuilder(const SegregatedListsAllocatorBuilder&) = delete;

	SegregatedListsAllocatorBuilder& operator=(const SegregatedListsAllocatorBuilder&) = delete;

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
	void free(TChunk* chunk, PtrSize offset, PtrSize size);

	/// Validate the internal structures. It's only used in testing.
	Error validate() const;

	/// Print debug info.
	void printFreeBlocks(StringListRaii& strList) const;

	/// It's 1-(largestBlockOfFreeMemory/totalFreeMemory). 0.0 is no fragmentation, 1.0 is totally fragmented.
	[[nodiscard]] F32 computeExternalFragmentation(PtrSize baseSize = 1) const;

	/// Adam Sawicki metric. 0.0 is no fragmentation, 1.0 is totally fragmented.
	[[nodiscard]] F32 computeExternalFragmentationSawicki(PtrSize baseSize = 1) const;

private:
	using FreeBlock = detail::SegregatedListsAllocatorBuilderFreeBlock;
	using ChunksIterator = typename DynamicArray<TChunk*>::Iterator;

	TInterface m_interface; ///< The interface.

	DynamicArray<TChunk*> m_chunks;

	mutable TLock m_lock;

	U32 findClass(PtrSize size, PtrSize alignment) const;

	/// Choose the best free block out of 2 given the allocation size and alignment.
	static Bool chooseBestFit(PtrSize allocSize, PtrSize allocAlignment, FreeBlock* blockA, FreeBlock* blockB,
							  FreeBlock*& bestBlock);

	/// Place a free block in one of the lists.
	/// @param[in,out] chunk The input chunk. If it's freed the pointer will become null.
	void placeFreeBlock(PtrSize address, PtrSize size, ChunksIterator chunkIt);
};
/// @}

} // end namespace anki

#include <AnKi/Util/SegregatedListsAllocatorBuilder.inl.h>
