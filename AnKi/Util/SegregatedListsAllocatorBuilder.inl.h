// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/SegregatedListsAllocatorBuilder.h>

namespace anki {

template<typename TChunk, typename TInterface, typename TLock>
SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::~SegregatedListsAllocatorBuilder()
{
	ANKI_ASSERT(m_chunks.isEmpty() && "Forgot to free memory");
}

template<typename TChunk, typename TInterface, typename TLock>
U32 SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::findClass(PtrSize size, PtrSize alignment) const
{
	ANKI_ASSERT(size > 0 && alignment > 0);

	for(U32 i = 0; i < TInterface::getClassCount(); ++i)
	{
		PtrSize maxSize;
		m_interface.getClassInfo(i, maxSize);

		if(size <= maxSize)
		{
			if(alignment <= maxSize)
			{
				// Found the class
				return i;
			}
			else
			{
				// The class found doesn't have the proper alignment. Need to go higher
			}
		}
	}

	// Not found
	return kMaxU32;
}

template<typename TChunk, typename TInterface, typename TLock>
Bool SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::chooseBestFit(PtrSize allocSize,
																			   PtrSize allocAlignment,
																			   FreeBlock* blockA, FreeBlock* blockB,
																			   FreeBlock*& bestBlock)
{
	ANKI_ASSERT(allocSize > 0 && allocAlignment > 0);
	ANKI_ASSERT(blockA || blockB);

	bestBlock = nullptr;

	PtrSize remainingSizeA = 0;
	if(blockA)
	{
		const PtrSize alignedAddress = getAlignedRoundUp(allocAlignment, blockA->m_address);
		const Bool fits = alignedAddress + allocSize <= blockA->m_address + blockA->m_size;

		if(fits)
		{
			bestBlock = blockA;
			remainingSizeA = blockA->m_size - allocSize - (alignedAddress - blockA->m_address);
		}
	}

	if(blockB)
	{
		const PtrSize alignedAddress = getAlignedRoundUp(allocAlignment, blockB->m_address);
		const Bool fits = alignedAddress + allocSize <= blockB->m_address + blockB->m_size;

		if(fits)
		{
			if(bestBlock)
			{
				const PtrSize remainingSizeB = blockB->m_size - allocSize - (alignedAddress - blockB->m_address);

				if(remainingSizeB < remainingSizeA)
				{
					bestBlock = blockB;
				}
			}
			else
			{
				bestBlock = blockB;
			}
		}
	}

	if(bestBlock)
	{
		return allocSize == bestBlock->m_size;
	}
	else
	{
		return false;
	}
}

template<typename TChunk, typename TInterface, typename TLock>
void SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::placeFreeBlock(PtrSize address, PtrSize size,
																				ChunksIterator chunkIt)
{
	ANKI_ASSERT(size > 0 && isAligned(m_interface.getMinSizeAlignment(), size));
	ANKI_ASSERT(chunkIt != m_chunks.getEnd());

	TChunk& chunk = *(*chunkIt);
	ANKI_ASSERT(address + size <= chunk.m_totalSize);

	// First search everything to find other coalesced free blocks
	U32 leftBlock = kMaxU32;
	U32 leftClass = kMaxU32;
	U32 rightBlock = kMaxU32;
	U32 rightClass = kMaxU32;
	for(U32 classIdx = 0; classIdx < TInterface::getClassCount(); ++classIdx)
	{
		const DynamicArray<FreeBlock>& freeLists = chunk.m_freeLists[classIdx];

		if(freeLists.getSize() == 0)
		{
			continue;
		}

		FreeBlock newBlock;
		newBlock.m_address = address;
		newBlock.m_size = size;
		auto it = std::lower_bound(freeLists.getBegin(), freeLists.getEnd(), newBlock,
								   [](const FreeBlock& a, const FreeBlock& b) {
									   return a.m_address + a.m_size < b.m_address;
								   });

		if(it == freeLists.getEnd())
		{
			continue;
		}

		const U32 pos = U32(it - freeLists.getBegin());
		const FreeBlock& freeBlock = freeLists[pos];

		if(freeBlock.m_address + freeBlock.m_size == address)
		{
			// Free block is in the left of the new

			ANKI_ASSERT(leftBlock == kMaxU32);
			leftBlock = pos;
			leftClass = classIdx;

			if(pos + 1 < freeLists.getSize())
			{
				const FreeBlock& freeBlock2 = freeLists[pos + 1];

				if(address + size == freeBlock2.m_address)
				{
					ANKI_ASSERT(rightBlock == kMaxU32);
					rightBlock = pos + 1;
					rightClass = classIdx;
				}
			}
		}
		else if(address + size == freeBlock.m_address)
		{
			// Free block is in the right of the new

			ANKI_ASSERT(rightBlock == kMaxU32);
			rightBlock = pos;
			rightClass = classIdx;

			if(pos > 0)
			{
				const FreeBlock& freeBlock2 = freeLists[pos - 1];

				if(freeBlock2.m_address + freeBlock2.m_size == address)
				{
					ANKI_ASSERT(leftBlock == kMaxU32);
					leftBlock = pos - 1;
					leftClass = classIdx;
				}
			}
		}
		else
		{
			// Do nothing
		}

		if(leftBlock != kMaxU32 && rightBlock != kMaxU32)
		{
			break;
		}
	}

	// Merge the new block
	FreeBlock newBlock;
	newBlock.m_address = address;
	newBlock.m_size = size;

	if(leftBlock != kMaxU32)
	{
		const FreeBlock& lblock = chunk.m_freeLists[leftClass][leftBlock];

		newBlock.m_address = lblock.m_address;
		newBlock.m_size += lblock.m_size;

		chunk.m_freeLists[leftClass].erase(m_interface.getMemoryPool(),
										   chunk.m_freeLists[leftClass].getBegin() + leftBlock);

		if(rightBlock != kMaxU32 && rightClass == leftClass)
		{
			// Both right and left blocks live in the same dynamic array. Due to the erase() above the rights's index
			// is no longer valid, adjust it
			ANKI_ASSERT(rightBlock > leftBlock);
			--rightBlock;
		}
	}

	if(rightBlock != kMaxU32)
	{
		const FreeBlock& rblock = chunk.m_freeLists[rightClass][rightBlock];

		newBlock.m_size += rblock.m_size;

		chunk.m_freeLists[rightClass].erase(m_interface.getMemoryPool(),
											chunk.m_freeLists[rightClass].getBegin() + rightBlock);
	}

	// Store the new block
	const U32 newClassIdx = findClass(newBlock.m_size, 1);
	chunk.m_freeLists[newClassIdx].emplaceBack(m_interface.getMemoryPool(), newBlock);

	std::sort(chunk.m_freeLists[newClassIdx].getBegin(), chunk.m_freeLists[newClassIdx].getEnd(),
			  [](const FreeBlock& a, const FreeBlock& b) {
				  return a.m_address < b.m_address;
			  });

	// Adjust chunk free size
	chunk.m_freeSize += size;
	ANKI_ASSERT(chunk.m_freeSize <= chunk.m_totalSize);
	if(chunk.m_freeSize == chunk.m_totalSize)
	{
		// Chunk completely free, delete it

		U32 blockCount = 0;
		for(U32 classIdx = 0; classIdx < TInterface::getClassCount(); ++classIdx)
		{
			blockCount += chunk.m_freeLists[classIdx].getSize();
			chunk.m_freeLists[classIdx].destroy(m_interface.getMemoryPool());
		}

		ANKI_ASSERT(blockCount == 1);

		m_chunks.erase(m_interface.getMemoryPool(), chunkIt);
		m_interface.deleteChunk(&chunk);
	}
}

template<typename TChunk, typename TInterface, typename TLock>
Error SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::allocate(PtrSize origSize, PtrSize origAlignment,
																		   TChunk*& outChunk, PtrSize& outOffset)
{
	ANKI_ASSERT(origSize > 0 && origAlignment > 0);
	const PtrSize size = getAlignedRoundUp(m_interface.getMinSizeAlignment(), origSize);
	const PtrSize alignment = max<PtrSize>(m_interface.getMinSizeAlignment(), origAlignment);

	// Find starting class
	const U32 startingClassIdx = findClass(size, alignment);
	if(ANKI_UNLIKELY(startingClassIdx == kMaxU32))
	{
		ANKI_UTIL_LOGE("Couldn't find class for allocation of size %zu", origSize);
		return Error::kOutOfMemory;
	}

	LockGuard<TLock> lock(m_lock);

	// Sort the chunks because we want to try allocate from the least empty first
	std::sort(m_chunks.getBegin(), m_chunks.getEnd(), [](const TChunk* a, const TChunk* b) {
		return a->m_freeSize < b->m_freeSize;
	});

	// Find a free block, its class and its chunk
	FreeBlock* freeBlock = nullptr;
	ChunksIterator chunkIt = m_chunks.getBegin();
	U32 classIdx = kMaxU32;
	for(; chunkIt != m_chunks.getEnd(); ++chunkIt)
	{
		classIdx = startingClassIdx;

		while(classIdx < TInterface::getClassCount())
		{
			// Find the best fit
			for(FreeBlock& block : (*chunkIt)->m_freeLists[classIdx])
			{
				const Bool theBestBlock = chooseBestFit(size, alignment, freeBlock, &block, freeBlock);
				if(theBestBlock)
				{
					break;
				}
			}

			if(freeBlock)
			{
				// Done
				break;
			}
			else
			{
				// Check for free blocks in the next class
				++classIdx;
			}
		}

		if(freeBlock)
		{
			break;
		}
	}

	if(freeBlock == nullptr)
	{
		// No free blocks, allocate new chunk

		// Init the new chunk
		PtrSize chunkSize;
		TChunk* chunk;
		ANKI_CHECK(m_interface.allocateChunk(chunk, chunkSize));

		if(chunkSize < size)
		{
			ANKI_UTIL_LOGE("Chunk allocated can't fit the current allocation of %zu", origSize);
			m_interface.deleteChunk(chunk);
			return Error::kOutOfMemory;
		}

		chunk->m_totalSize = chunkSize;
		chunk->m_freeSize = 0;
		m_chunks.emplaceBack(m_interface.getMemoryPool(), chunk);

		placeFreeBlock(size, chunkSize - size, m_chunks.getBegin() + m_chunks.getSize() - 1);

		// Allocate
		outChunk = chunk;
		outOffset = 0;
	}
	else
	{
		// Have a free block, allocate from it

		ANKI_ASSERT(chunkIt != m_chunks.getEnd() && freeBlock);

		TChunk& chunk = *(*chunkIt);

		const FreeBlock fBlock = *freeBlock;
		chunk.m_freeLists[classIdx].erase(m_interface.getMemoryPool(), freeBlock);
		freeBlock = nullptr;

		ANKI_ASSERT(chunk.m_freeSize >= fBlock.m_size);
		chunk.m_freeSize -= fBlock.m_size;

		const PtrSize alignedAddress = getAlignedRoundUp(alignment, fBlock.m_address);

		if(alignedAddress != fBlock.m_address)
		{
			// Add a free block because of alignment missmatch
			placeFreeBlock(fBlock.m_address, alignedAddress - fBlock.m_address, chunkIt);
		}

		const PtrSize allocationEnd = alignedAddress + size;
		const PtrSize freeBlockEnd = fBlock.m_address + fBlock.m_size;
		if(allocationEnd < freeBlockEnd)
		{
			// Add what remains
			placeFreeBlock(allocationEnd, freeBlockEnd - allocationEnd, chunkIt);
		}

		// Allocate
		outChunk = &chunk;
		outOffset = alignedAddress;
	}

	ANKI_ASSERT(outChunk);
	ANKI_ASSERT(isAligned(alignment, outOffset));
	return Error::kNone;
}

template<typename TChunk, typename TInterface, typename TLock>
void SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::free(TChunk* chunk, PtrSize offset, PtrSize size)
{
	ANKI_ASSERT(chunk && size);

	LockGuard<TLock> lock(m_lock);

	ChunksIterator it = m_chunks.getBegin();
	for(; it != m_chunks.getEnd(); ++it)
	{
		if(*it == chunk)
		{
			break;
		}
	}
	ANKI_ASSERT(it != m_chunks.getEnd());

	size = getAlignedRoundUp(m_interface.getMinSizeAlignment(), size);

	placeFreeBlock(offset, size, it);
}

template<typename TChunk, typename TInterface, typename TLock>
Error SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::validate() const
{
#define ANKI_SLAB_ASSERT(x, ...) \
	do \
	{ \
		if(ANKI_UNLIKELY(!(x))) \
		{ \
			ANKI_UTIL_LOGE(__VA_ARGS__); \
			ANKI_DEBUG_BREAK(); \
			return Error::kFunctionFailed; \
		} \
	} while(0)

	LockGuard<TLock> lock(m_lock);

	U32 chunkIdx = 0;
	for(const TChunk* chunk : m_chunks)
	{
		ANKI_SLAB_ASSERT(chunk->m_totalSize > 0, "Chunk %u: Total size can't be 0", chunkIdx);
		ANKI_SLAB_ASSERT(chunk->m_freeSize < chunk->m_totalSize,
						 "Chunk %u: Free size (%zu) should be less than total size (%zu)", chunkIdx, chunk->m_freeSize,
						 chunk->m_totalSize);

		PtrSize freeSize = 0;
		for(U32 c = 0; c < TInterface::getClassCount(); ++c)
		{
			for(U32 i = 0; i < chunk->m_freeLists[c].getSize(); ++i)
			{
				const FreeBlock& crnt = chunk->m_freeLists[c][i];
				ANKI_SLAB_ASSERT(crnt.m_size > 0, "Chunk %u class %u block %u: Block size can't be 0", chunkIdx, c, i);
				freeSize += crnt.m_size;

				const U32 classIdx = findClass(crnt.m_size, 1);
				ANKI_SLAB_ASSERT(classIdx == c, "Chunk %u class %u block %u: Free block not in the correct class",
								 chunkIdx, c, i);

				if(i > 0)
				{
					const FreeBlock& prev = chunk->m_freeLists[c][i - 1];
					ANKI_SLAB_ASSERT(prev.m_address < crnt.m_address,
									 "Chunk %u class %u block %u: Previous block should have lower address", chunkIdx,
									 c, i);
					ANKI_SLAB_ASSERT(prev.m_address + prev.m_size < crnt.m_address,
									 "Chunk %u class %u block %u: Block overlaps with previous", chunkIdx, c, i);
				}
			}
		}

		ANKI_SLAB_ASSERT(freeSize == chunk->m_freeSize,
						 "Chunk %u: Free size calculated doesn't match chunk's free size", chunkIdx);
		++chunkIdx;
	}

	// Now do an expensive check that blocks never overlap with other blocks of other classes
	chunkIdx = 0;
	for(const TChunk* chunk : m_chunks)
	{
		for(U32 c = 0; c < TInterface::getClassCount(); ++c)
		{
			for(U32 i = 0; i < chunk->m_freeLists[c].getSize(); ++i)
			{
				const FreeBlock& crnt = chunk->m_freeLists[c][i];

				for(U32 c2 = 0; c2 < TInterface::getClassCount(); ++c2)
				{
					for(U32 j = 0; j < chunk->m_freeLists[c2].getSize(); ++j)
					{
						if(c == c2 && i == j)
						{
							// Skip "crnt"
							continue;
						}

						const FreeBlock& other = chunk->m_freeLists[c2][j];

						// Check coalescing
						if(crnt.m_address < other.m_address)
						{
							ANKI_SLAB_ASSERT(crnt.m_address + crnt.m_size < other.m_address,
											 "Chunk %u class %u block %u: Block overlaps or should have been merged "
											 "with block: class %u block %u",
											 chunkIdx, c, i, c2, j);
						}
						else if(crnt.m_address > other.m_address)
						{
							ANKI_SLAB_ASSERT(other.m_address + other.m_size < crnt.m_address,
											 "Chunk %u class %u block %u: Block overlaps or should have been merged "
											 "with block: class %u block %u",
											 chunkIdx, c, i, c2, j);
						}
						else
						{
							ANKI_SLAB_ASSERT(false,
											 "Chunk %u class %u block %u: Block shouldn't be having the same address "
											 "with: class %u block %u",
											 chunkIdx, c, i, c2, j);
						}
					}
				}
			}
		}

		++chunkIdx;
	}

#undef ANKI_SLAB_ASSERT
	return Error::kNone;
}

template<typename TChunk, typename TInterface, typename TLock>
void SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::printFreeBlocks(StringListRaii& strList) const
{
	LockGuard<TLock> lock(m_lock);

	U32 chunkCount = 0;
	for(const TChunk* chunk : m_chunks)
	{
		strList.pushBackSprintf("Chunk #%u, total size %zu, free size %zu\n", chunkCount, chunk->m_totalSize,
								chunk->m_freeSize);

		for(U32 c = 0; c < TInterface::getClassCount(); ++c)
		{
			if(chunk->m_freeLists[c].getSize())
			{
				strList.pushBackSprintf("  Class #%u\n    ", c);
			}

			for(U32 i = 0; i < chunk->m_freeLists[c].getSize(); ++i)
			{
				const FreeBlock& blk = chunk->m_freeLists[c][i];
				strList.pushBackSprintf("| %zu-%zu(%zu) ", blk.m_address, blk.m_address + blk.m_size - 1, blk.m_size);
			}

			if(chunk->m_freeLists[c].getSize())
			{
				strList.pushBack("|\n");
			}
		}

		++chunkCount;
	}
}

template<typename TChunk, typename TInterface, typename TLock>
F32 SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::computeExternalFragmentation(PtrSize baseSize) const
{
	ANKI_ASSERT(baseSize > 0);

	LockGuard<TLock> lock(m_lock);

	F32 maxFragmentation = 0.0f;

	for(const TChunk* chunk : m_chunks)
	{
		PtrSize largestFreeBlockSize = 0;

		for(U32 c = 0; c < TInterface::getClassCount(); ++c)
		{
			for(const FreeBlock& block : chunk->m_freeLists[c])
			{
				largestFreeBlockSize = max(largestFreeBlockSize, block.m_size / baseSize);
			}
		}

		const F32 frag = F32(1.0 - F64(largestFreeBlockSize) / F64(chunk->m_freeSize / baseSize));

		maxFragmentation = max(maxFragmentation, frag);
	}

	return maxFragmentation;
}

template<typename TChunk, typename TInterface, typename TLock>
F32 SegregatedListsAllocatorBuilder<TChunk, TInterface, TLock>::computeExternalFragmentationSawicki(
	PtrSize baseSize) const
{
	ANKI_ASSERT(baseSize > 0);

	LockGuard<TLock> lock(m_lock);

	F32 maxFragmentation = 0.0f;

	for(const TChunk* chunk : m_chunks)
	{
		F64 quality = 0.0;

		for(U32 c = 0; c < TInterface::getClassCount(); ++c)
		{
			for(const FreeBlock& block : chunk->m_freeLists[c])
			{
				const F64 size = F64(block.m_size / baseSize);
				quality += size * size;
			}
		}

		quality = sqrt(quality) / F64(chunk->m_freeSize / baseSize);
		const F32 frag = 1.0f - F32(quality * quality);

		maxFragmentation = max(maxFragmentation, frag);
	}

	return maxFragmentation;
}

} // end namespace anki
