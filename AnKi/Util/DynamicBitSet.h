// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/DynamicArray.h>

namespace anki {

// A dynamic array of bits.
template<typename TMemoryPool, typename TElementType>
class DynamicBitSet
{
public:
	DynamicBitSet() = default;

	DynamicBitSet(const DynamicBitSet& b)
		: m_storage(b.m_storage)
	{
	}

	DynamicBitSet(DynamicBitSet&& b)
		: m_storage(std::move(b.m_storage))
	{
	}

	DynamicBitSet& operator=(const DynamicBitSet& b)
	{
		m_storage = b.m_storage;
		return *this;
	}

	DynamicBitSet& operator=(DynamicBitSet&& b)
	{
		m_storage = std::move(b.m_storage);
		return *this;
	}

	DynamicBitSet& setBit(U32 pos)
	{
		U32 index, bit;
		decode(pos, index, bit);
		if(index >= m_storage.getSize())
		{
			m_storage.resize(index + 1, 0u);
		}

		m_storage[index] |= TElementType(1) << TElementType(bit);
		return *this;
	}

	Bool getBit(U32 pos) const
	{
		U32 index, bit;
		decode(pos, index, bit);
		if(index >= m_storage.getSize())
		{
			return false;
		}

		const TElementType mask = TElementType(1) << TElementType(bit);
		return (m_storage[index] & mask) != 0;
	}

private:
	DynamicArray<TElementType, TMemoryPool> m_storage;

	static void decode(U32 pos, U32& index, U32& bit)
	{
		index = pos / sizeof(TElementType);
		bit = pos % sizeof(TElementType);
	}
};

} // end namespace anki
