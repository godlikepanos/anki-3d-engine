// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/util/List.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Manages pre-allocated, always mapped GPU memory in blocks.
class GpuBlockAllocator : public NonCopyable
{
public:
	/// Default constructor.
	GpuBlockAllocator()
	{
	}

	/// Destructor.
	~GpuBlockAllocator();

	/// Initialize the allocator using pre-allocated CPU mapped memory.
	void init(GenericMemoryPoolAllocator<U8> alloc, PtrSize totalSize, PtrSize blockSize);

	/// Allocate GPU memory.
	ANKI_USE_RESULT Error allocate(PtrSize size, U alignment, PtrSize& outOffset);

	/// Free GPU memory.
	void free(PtrSize offset);

private:
	class Block;

	GenericMemoryPoolAllocator<U8> m_alloc;
	PtrSize m_size = 0;
	PtrSize m_blockSize = 0;
	DynamicArray<Block> m_blocks;

	DynamicArray<U32> m_freeBlocksStack;
	U32 m_freeBlockCount = 0;
	U32 m_currentBlock = MAX_U32;
	Mutex m_mtx;

	Bool isCreated() const
	{
		return m_size > 0;
	}

	Bool blockHasEnoughSpace(U blockIdx, PtrSize size, U alignment) const;
};
/// @}

} // end namespace anki