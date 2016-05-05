// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/GpuBlockAllocator.h>

namespace anki
{

//==============================================================================
// GpuBlockAllocator::Block                                                    =
//==============================================================================
class GpuBlockAllocator::Block
	: public IntrusiveListEnabled<GpuBlockAllocator::Block>
{
public:
	PtrSize m_offset = 0;
	U32 m_allocationCount = 0;
};

//==============================================================================
// GpuBlockAllocator                                                           =
//==============================================================================

//==============================================================================
GpuBlockAllocator::~GpuBlockAllocator()
{
	if(m_freeBlockCount != m_blocks.getSize())
	{
		ANKI_LOGW("Forgot to free memory");
	}

	m_blocks.destroy(m_alloc);
	m_freeBlocksStack.destroy(m_alloc);
}

//==============================================================================
void GpuBlockAllocator::init(GenericMemoryPoolAllocator<U8> alloc,
	void* cpuMappedMem,
	PtrSize cpuMappedMemSize,
	PtrSize blockSize)
{
	ANKI_ASSERT(cpuMappedMem && cpuMappedMemSize > 0 && blockSize > 0);
	ANKI_ASSERT((cpuMappedMemSize % blockSize) == 0);

	m_alloc = alloc;
	m_mem = static_cast<U8*>(cpuMappedMem);
	m_size = cpuMappedMemSize;
	m_blockSize = blockSize;
	m_blocks.create(alloc, cpuMappedMemSize / blockSize);

	m_freeBlocksStack.create(alloc, m_blocks.getSize());
	m_freeBlockCount = m_blocks.getSize();
	m_currentBlock = MAX_U32;

	U count = m_freeBlocksStack.getSize();
	for(U32& i : m_freeBlocksStack)
	{
		i = --count;
	}
}

//==============================================================================
Bool GpuBlockAllocator::blockHasEnoughSpace(
	U blockIdx, PtrSize size, U alignment) const
{
	ANKI_ASSERT(size > 0);

	const Block& block = m_blocks[blockIdx];

	U8* allocEnd = getAlignedRoundUp(alignment, m_mem + block.m_offset) + size;
	U8* blockEnd = m_mem + blockIdx * m_blockSize + m_blockSize;

	return allocEnd <= blockEnd;
}

//==============================================================================
void* GpuBlockAllocator::allocate(
	PtrSize size, U alignment, DynamicBufferToken& handle)
{
	ANKI_ASSERT(size < m_blockSize);

	Block* block = nullptr;
	U8* ptr = nullptr;

	LockGuard<Mutex> lock(m_mtx);

	if(m_currentBlock == MAX_U32
		|| !blockHasEnoughSpace(m_currentBlock, size, alignment))
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
		ptr = getAlignedRoundUp(alignment, m_mem + block->m_offset);

		PtrSize outOffset = ptr - m_mem;
		block->m_offset = outOffset + size;
		ANKI_ASSERT(
			block->m_offset <= (block - &m_blocks[0] + 1) * m_blockSize);

		++block->m_allocationCount;

		// Update the handle
		handle.m_offset = outOffset;
		handle.m_range = size;
	}

	return static_cast<void*>(ptr);
}

//==============================================================================
void GpuBlockAllocator::free(void* vptr)
{
	U8* ptr = static_cast<U8*>(vptr);
	ANKI_ASSERT(ptr);
	ANKI_ASSERT(ptr >= m_mem && ptr < m_mem + m_size);

	PtrSize offset = static_cast<PtrSize>(ptr - m_mem);
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