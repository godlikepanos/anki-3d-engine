// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/GpuBlockAllocator.h>

namespace anki
{

class GpuBlockAllocator::Block : public IntrusiveListEnabled<GpuBlockAllocator::Block>
{
public:
	PtrSize m_offset = 0;
	U32 m_allocationCount = 0;
};

GpuBlockAllocator::~GpuBlockAllocator()
{
	if(m_freeBlockCount != m_blocks.getSize())
	{
		ANKI_LOGW("Forgot to free memory");
	}

	m_blocks.destroy(m_alloc);
	m_freeBlocksStack.destroy(m_alloc);
}

void GpuBlockAllocator::init(GenericMemoryPoolAllocator<U8> alloc, PtrSize totalSize, PtrSize blockSize)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(totalSize > 0 && blockSize > 0);
	ANKI_ASSERT(totalSize > blockSize);
	ANKI_ASSERT((totalSize % blockSize) == 0);

	m_alloc = alloc;
	m_size = totalSize;
	m_blockSize = blockSize;
	m_blocks.create(alloc, totalSize / blockSize);

	m_freeBlocksStack.create(alloc, m_blocks.getSize());
	m_freeBlockCount = m_blocks.getSize();
	m_currentBlock = MAX_U32;

	U count = m_freeBlocksStack.getSize();
	for(U32& i : m_freeBlocksStack)
	{
		i = --count;
	}
}

Bool GpuBlockAllocator::blockHasEnoughSpace(U blockIdx, PtrSize size, U alignment) const
{
	ANKI_ASSERT(size > 0);

	const Block& block = m_blocks[blockIdx];

	PtrSize allocEnd = getAlignedRoundUp(alignment, block.m_offset) + size;
	PtrSize blockEnd = (blockIdx + 1) * m_blockSize;

	return allocEnd <= blockEnd;
}

Error GpuBlockAllocator::allocate(PtrSize size, U alignment, PtrSize& outOffset)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(size < m_blockSize);

	Block* block = nullptr;
	Error err = ErrorCode::NONE;

	LockGuard<Mutex> lock(m_mtx);

	if(m_currentBlock == MAX_U32 || !blockHasEnoughSpace(m_currentBlock, size, alignment))
	{
		// Need new block
		if(m_freeBlockCount > 0)
		{
			// Pop block from free
			U blockIdx = --m_freeBlockCount;

			block = &m_blocks[blockIdx];
			block->m_offset = blockIdx * m_blockSize;
			ANKI_ASSERT(block->m_allocationCount == 0);

			// Make it in-use
			m_currentBlock = blockIdx;
		}
	}

	if(block)
	{
		PtrSize outOffset = getAlignedRoundUp(alignment, block->m_offset);
		block->m_offset = outOffset + size;
		ANKI_ASSERT(block->m_offset <= (block - &m_blocks[0] + 1) * m_blockSize);

		++block->m_allocationCount;

		// Update the handle
		outOffset = outOffset;
	}
	else
	{
		err = ErrorCode::OUT_OF_MEMORY;
		outOffset = MAX_PTR_SIZE;
	}

	return err;
}

void GpuBlockAllocator::free(PtrSize offset)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(offset < m_size);

	U blockIdx = offset / m_blockSize;

	LockGuard<Mutex> lock(m_mtx);

	ANKI_ASSERT(m_blocks[blockIdx].m_allocationCount > 0);
	if((--m_blocks[blockIdx].m_allocationCount) == 0)
	{
		// Block no longer in use

		if(m_currentBlock == blockIdx)
		{
			m_currentBlock = MAX_U32;
		}

		m_freeBlocksStack[m_freeBlockCount++] = blockIdx;
	}
}

} // end namespace anki
