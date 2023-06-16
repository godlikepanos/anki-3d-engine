// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/BlockArray.h>

namespace anki {

template<typename T, typename TMemoryPool, typename TConfig>
void BlockArray<T, TMemoryPool, TConfig>::destroy()
{
	for(U32 i = 0; i < m_blockStorages.getSize(); ++i)
	{
		U32 localIdx;
		while((localIdx = m_blockMetadatas[i].m_elementsInUseMask.getLeastSignificantBit()) != kMaxU32)
		{
			reinterpret_cast<T*>(&m_blockStorages[i]->m_storage[localIdx * sizeof(T)])->~T();
			m_blockMetadatas[i].m_elementsInUseMask.unset(localIdx);
		}

		if(m_blockStorages[i])
		{
			getMemoryPool().free(m_blockStorages[i]);
		}
	}

	m_blockMetadatas.destroy();
	m_blockStorages.destroy();
	m_elementCount = 0;
}

template<typename T, typename TMemoryPool, typename TConfig>
template<typename... TArgs>
BlockArray<T, TMemoryPool, TConfig>::Iterator BlockArray<T, TMemoryPool, TConfig>::emplace(TArgs&&... args)
{
	U32 localIdx = kMaxU32;
	U32 blockIdx = kMaxU32;

	// Search for a block with free elements
	for(U32 i = 0; i < m_blockStorages.getSize(); ++i)
	{
		if(m_blockMetadatas[i].m_elementsInUseMask.getEnabledBitCount() < kElementCountPerBlock)
		{
			// Found a block, allocate from it
			auto unsetBits = ~m_blockMetadatas[i].m_elementsInUseMask;
			localIdx = unsetBits.getLeastSignificantBit();
			blockIdx = i;

			if(m_blockStorages[blockIdx] == nullptr)
			{
				m_blockStorages[blockIdx] = newInstance<BlockStorage>(getMemoryPool());
			}

			break;
		}
	}

	// Block not found, crate new
	if(blockIdx == kMaxU32)
	{
		m_blockMetadatas.emplaceBack(false);
		m_blockStorages.emplaceBack(newInstance<BlockStorage>(getMemoryPool()));

		localIdx = 0;
		blockIdx = m_blockMetadatas.getSize() - 1;
	}

	// Finalize and return
	ANKI_ASSERT(localIdx < kElementCountPerBlock);

	::new(&m_blockStorages[blockIdx]->m_storage[localIdx * sizeof(T)]) T(std::forward<TArgs>(args)...);

	ANKI_ASSERT(m_blockMetadatas[blockIdx].m_elementsInUseMask.get(localIdx) == false);
	m_blockMetadatas[blockIdx].m_elementsInUseMask.set(localIdx);

#if ANKI_EXTRA_CHECKS
	++m_iteratorVer;
#endif
	++m_elementCount;

	return Iterator(this, blockIdx * kElementCountPerBlock + localIdx
#if ANKI_EXTRA_CHECKS
					,
					m_iteratorVer
#endif
	);
}

template<typename T, typename TMemoryPool, typename TConfig>
void BlockArray<T, TMemoryPool, TConfig>::erase(ConstIterator at)
{
	const U32 idx = at.m_elementIdx;
	const U32 localIdx = idx % kElementCountPerBlock;
	const U32 blockIdx = idx / kElementCountPerBlock;
	ANKI_ASSERT(blockIdx < m_blockStorages.getSize());

	Mask& inUseMask = m_blockMetadatas[blockIdx].m_elementsInUseMask;
	ANKI_ASSERT(inUseMask.get(localIdx) == true);
	BlockStorage* block = m_blockStorages[blockIdx];
	ANKI_ASSERT(block);

	reinterpret_cast<T*>(&block->m_storage[localIdx * sizeof(T)])->~T();

	inUseMask.unset(localIdx);
	if(inUseMask.getEnabledBitCount() == 0)
	{
		// Block is empty, delete it
		getMemoryPool().free(block);
		m_blockStorages[blockIdx] = nullptr;
	}

	ANKI_ASSERT(m_elementCount > 0);
	--m_elementCount;
}

template<typename T, typename TMemoryPool, typename TConfig>
BlockArray<T, TMemoryPool, TConfig>& BlockArray<T, TMemoryPool, TConfig>::operator=(const BlockArray& b)
{
	destroy();

	if(b.m_elementCount == 0)
	{
		return *this;
	}

	m_elementCount = b.m_elementCount;
	m_blockStorages.resize(b.m_blockStorages.getSize());
	m_blockMetadatas.resize(b.m_blockMetadatas.getSize());
#if ANKI_EXTRA_CHECKS
	++m_iteratorVer;
#endif

	for(U32 blockIdx = 0; blockIdx < b.m_blockMetadatas.getSize(); ++blockIdx)
	{
		Mask mask = b.m_blockMetadatas[blockIdx].m_elementsInUseMask;
		U32 localIdx;
		while((localIdx = mask.getLeastSignificantBit()) != kMaxU32)
		{
			const T& other = b[blockIdx * kElementCountPerBlock + localIdx];
			::new(&b.m_blockStorages[blockIdx].m_storage[localIdx * sizeof(T)]) T(other);
			mask.unset(localIdx);
		}
	}

	return *this;
}

} // end namespace anki
