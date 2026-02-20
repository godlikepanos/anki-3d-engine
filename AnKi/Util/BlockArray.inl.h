// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/BlockArray.h>

namespace anki {

template<typename T, typename TConfig, typename TMemoryPool>
void BlockArray<T, TConfig, TMemoryPool>::destroy()
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
	m_firstIndex = 0;
	m_endIndex = 0;
}

template<typename T, typename TConfig, typename TMemoryPool>
template<typename... TArgs>
typename BlockArray<T, TConfig, TMemoryPool>::Iterator BlockArray<T, TConfig, TMemoryPool>::emplace(TArgs&&... args)
{
	U32 localIdx = kMaxU32;
	U32 blockIdx = kMaxU32;

	// Search for a block with free elements
	for(U32 i = 0; i < m_blockStorages.getSize(); ++i)
	{
		if(m_blockMetadatas[i].m_elementsInUseMask.getSetBitCount() < kElementCountPerBlock)
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

	++m_elementCount;
	const U32 idx = blockIdx * kElementCountPerBlock + localIdx;
	m_firstIndex = min(m_firstIndex, idx);
	m_endIndex = max(m_endIndex, idx + 1);

	return Iterator(this, idx);
}

template<typename T, typename TConfig, typename TMemoryPool>
void BlockArray<T, TConfig, TMemoryPool>::erase(Iterator it)
{
	const U32 idx = it.getArrayIndex();
	const U32 localIdx = idx % kElementCountPerBlock;
	const U32 blockIdx = idx / kElementCountPerBlock;

	ANKI_ASSERT(blockIdx < m_blockStorages.getSize());
	BlockStorage* block = m_blockStorages[blockIdx];
	ANKI_ASSERT(block);

	Mask& inUseMask = m_blockMetadatas[blockIdx].m_elementsInUseMask;
	ANKI_ASSERT(inUseMask.get(localIdx) == true);

	reinterpret_cast<T*>(&block->m_storage[localIdx * sizeof(T)])->~T();

	inUseMask.unset(localIdx);
	if(inUseMask.getSetBitCount() == 0)
	{
		// Block is empty, delete it
		getMemoryPool().free(block);
		m_blockStorages[blockIdx] = nullptr;
	}

	ANKI_ASSERT(m_elementCount > 0);
	--m_elementCount;

	if(m_elementCount == 0)
	{
		m_firstIndex = 0;
		m_endIndex = 0;
		m_blockStorages.destroy();
		m_blockMetadatas.destroy();
	}
	else
	{
		if(idx == m_firstIndex)
		{
			m_firstIndex = getFirstElementIndex();
		}

		if(idx + 1 == m_endIndex)
		{
			m_endIndex = getLastElementIndex() + 1;
		}
	}
}

template<typename T, typename TConfig, typename TMemoryPool>
BlockArray<T, TConfig, TMemoryPool>& BlockArray<T, TConfig, TMemoryPool>::operator=(const BlockArray& b)
{
	destroy();

	if(b.m_elementCount == 0)
	{
		return *this;
	}

	m_elementCount = b.m_elementCount;
	m_firstIndex = b.m_firstIndex;
	m_endIndex = b.m_endIndex;
	m_blockMetadatas = b.m_blockMetadatas;
	m_blockStorages.resize(b.m_blockStorages.getSize());

	for(U32 blockIdx = 0; blockIdx < b.m_blockMetadatas.getSize(); ++blockIdx)
	{
		Mask mask = b.m_blockMetadatas[blockIdx].m_elementsInUseMask;
		if(mask.getAnySet())
		{
			m_blockStorages[blockIdx] = newInstance<BlockStorage>(getMemoryPool());

			U32 localIdx;
			while((localIdx = mask.getLeastSignificantBit()) != kMaxU32)
			{
				const T& other = b[blockIdx * kElementCountPerBlock + localIdx];
				::new(&m_blockStorages[blockIdx]->m_storage[localIdx * sizeof(T)]) T(other);
				mask.unset(localIdx);
			}
		}
		else
		{
			m_blockStorages[blockIdx] = nullptr;
		}
	}

	return *this;
}

template<typename T, typename TConfig, typename TMemoryPool>
U32 BlockArray<T, TConfig, TMemoryPool>::getNextElementIndex(U32 crnt) const
{
	ANKI_ASSERT(crnt < m_endIndex);

	++crnt;
	for(; crnt < m_endIndex; ++crnt)
	{
		const U32 localIdx = crnt % kElementCountPerBlock;
		const U32 blockIdx = crnt / kElementCountPerBlock;

		if(m_blockMetadatas[blockIdx].m_elementsInUseMask.get(localIdx))
		{
			return crnt;
		}
	}

	return m_endIndex;
}

template<typename T, typename TConfig, typename TMemoryPool>
void BlockArray<T, TConfig, TMemoryPool>::validate() const
{
	ANKI_ASSERT(m_blockStorages.getSize() == m_blockMetadatas.getSize());

	[[maybe_unused]] U32 count = 0;
	U32 first = 0;
	U32 end = 0;
	for(U32 i = 0; i < m_blockStorages.getSize(); ++i)
	{
		const Mask& mask = m_blockMetadatas[i].m_elementsInUseMask;
		const U32 lcount = mask.getSetBitCount();
		if(lcount == 0)
		{
			ANKI_ASSERT(m_blockStorages[i] == nullptr);
		}
		else
		{
			count += lcount;
			first = min(first, mask.getLeastSignificantBit() + i * kElementCountPerBlock);
			end = max(end, mask.getMostSignificantBit() + i * kElementCountPerBlock + 1);
		}
	}

	ANKI_ASSERT(count == m_elementCount);
	ANKI_ASSERT(first == m_firstIndex);
	ANKI_ASSERT(end == m_endIndex);
}

} // end namespace anki
