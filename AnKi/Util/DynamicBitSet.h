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
	DynamicBitSet(const TMemoryPool& pool = TMemoryPool())
		: m_storage(pool)
	{
	}

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

	DynamicBitSet& setBit(U32 pos, Bool setBit = true)
	{
		U32 index, bit;
		decode(pos, index, bit);
		if(index >= m_storage.getSize())
		{
			m_storage.resize(index + 1, 0u);
		}

		if(setBit)
		{
			m_storage[index] |= TElementType(1) << TElementType(bit);
		}
		else
		{
			m_storage[index] &= ~(TElementType(1) << TElementType(bit));
		}
		return *this;
	}

	DynamicBitSet& unsetBit(U32 pos)
	{
		return setBit(pos, false);
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

	U32 getSetBitCount() const
	{
		U32 count = 0;
		for(TElementType el : m_storage)
		{
			count += std::popcount(el);
		}
		return count;
	}

	// Get the most significant bit that is enabled. Or kMaxU32 if all is zero.
	U32 getMostSignificantBit() const
	{
		for(I32 i = I32(m_storage.getSize()) - 1; i >= 0; --i)
		{
			if(m_storage[i] != 0)
			{
				const U32 msb = kElementBitCount - 1u - U32(std::countl_zero(m_storage[i]));
				return msb + U32(i) * kElementBitCount;
			}
		}

		return kMaxU32;
	}

	// Get the least significant bit that is enabled. Or kMaxU32 if all is zero.
	U32 getLeastSignificantBit() const
	{
		for(U32 i = 0; i < m_storage.getSize(); ++i)
		{
			if(m_storage[i] != 0)
			{
				return U32(std::countr_zero(m_storage[i])) + i * kElementBitCount;
			}
		}

		return kMaxU32;
	}

	template<typename TFunc>
	FunctorContinue iterateSetBitsFromLeastSignificant(TFunc func) const
	{
		for(U32 i = 0; i < m_storage.getSize(); ++i)
		{
			TElementType bits = m_storage[i];
			while(bits)
			{
				const U32 lsb = U32(std::countr_zero(bits));
				const U32 bitIdx = lsb + (i * kElementBitCount);

				const FunctorContinue cont = func(bitIdx);
				if(cont == FunctorContinue::kStop)
				{
					return cont;
				}

				bits &= ~(TElementType(1) << TElementType(lsb));
			}
		}

		return FunctorContinue::kContinue;
	}

	void destroy()
	{
		m_storage.destroy();
	}

private:
	static const U32 kElementBitCount = sizeof(TElementType) * 8;

	DynamicArray<TElementType, TMemoryPool> m_storage;

	static void decode(U32 pos, U32& index, U32& bit)
	{
		index = pos / kElementBitCount;
		bit = pos % kElementBitCount;
	}
};

} // end namespace anki
