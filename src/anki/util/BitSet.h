// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <initializer_list>
#include <cstring>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// Easy bit manipulation.
/// @tparam N The number of bits.
/// @tparam TChunkType The type of the chunks that the bitset consists. By default it's U8.
template<U N, typename TChunkType = U8>
class BitSet
{
public:
	/// Constructor. It will set all the bits or unset them.
	BitSet(Bool set)
	{
		if(!set)
		{
			unsetAll();
		}
		else
		{
			setAll();
		}
	}

	/// Bitwise or between this and @a b sets.
	BitSet operator|(const BitSet& b) const
	{
		BitSet out;
		for(U i = 0; i < CHUNK_COUNT; ++i)
		{
			out.m_chunks[i] = m_chunks[i] | b.m_chunks[i];
		}
		return out;
	}

	/// Bitwise or between this and @a b sets.
	BitSet& operator|=(const BitSet& b)
	{
		for(U i = 0; i < CHUNK_COUNT; ++i)
		{
			m_chunks[i] |= b.m_chunks[i];
		}
		return *this;
	}

	/// Bitwise and between this and @a b sets.
	BitSet operator&(const BitSet& b) const
	{
		BitSet out;
		for(U i = 0; i < CHUNK_COUNT; ++i)
		{
			out.m_chunks[i] = m_chunks[i] & b.m_chunks[i];
		}
		return out;
	}

	/// Bitwise and between this and @a b sets.
	BitSet& operator&=(const BitSet& b)
	{
		for(U i = 0; i < CHUNK_COUNT; ++i)
		{
			m_chunks[i] &= b.m_chunks[i];
		}
		return *this;
	}

	/// Bitwise xor between this and @a b sets.
	BitSet operator^(const BitSet& b) const
	{
		BitSet out;
		for(U i = 0; i < CHUNK_COUNT; ++i)
		{
			out.m_chunks[i] = m_chunks[i] ^ b.m_chunks[i];
		}
		return out;
	}

	/// Bitwise xor between this and @a b sets.
	BitSet& operator^=(const BitSet& b)
	{
		for(U i = 0; i < CHUNK_COUNT; ++i)
		{
			m_chunks[i] ^= b.m_chunks[i];
		}
		return *this;
	}

	Bool operator==(const BitSet& b) const
	{
		return memcmp(&m_chunks[0], &b.m_chunks[0], sizeof(m_chunks)) == 0;
	}

	Bool operator!=(const BitSet& b) const
	{
		return !(*this == b);
	}

	Bool operator!() const
	{
		return !getAny();
	}

	/// Set or unset a bit at the given position.
	template<typename TInt>
	void set(TInt pos, Bool setBits = true)
	{
		U high, low;
		position(static_cast<U>(pos), high, low);
		ChunkType mask = MASK >> low;
		m_chunks[high] = (setBits) ? (m_chunks[high] | mask) : (m_chunks[high] & ~mask);
	}

	/// Set multiple bits.
	template<typename TInt>
	void set(std::initializer_list<TInt> list, Bool setBits = true)
	{
		for(auto it : list)
		{
			set(it, setBits);
		}
	}

	/// Set all bits.
	void setAll()
	{
		memset(m_chunks, 0xFF, sizeof(m_chunks));

		// Zero the unused bits
		const ChunkType REMAINING_BITS = N - (CHUNK_COUNT - 1) * CHUNK_BIT_COUNT;
		const ChunkType REMAINING_BITMASK = std::numeric_limits<ChunkType>::max() >> REMAINING_BITS;
		m_chunks[CHUNK_COUNT - 1] ^= REMAINING_BITMASK;
	}

	/// Unset a bit (set to zero) at the given position.
	template<typename TInt>
	void unset(TInt pos)
	{
		set(pos, false);
	}

	/// Unset multiple bits.
	template<typename TInt>
	void unset(std::initializer_list<TInt> list)
	{
		set(list, false);
	}

	/// Unset all bits.
	void unsetAll()
	{
		memset(m_chunks, 0, sizeof(m_chunks));
	}

	/// Flip the bits at the given position. It will go from 1 to 0 or from 0 to 1.
	template<typename TInt>
	void flip(TInt pos)
	{
		U high, low;
		position(static_cast<U>(pos), high, low);
		ChunkType mask = MASK >> low;
		m_chunks[high] ^= mask;
	}

	/// Return true if the bit is set or false if it's not.
	template<typename TInt>
	Bool get(TInt pos) const
	{
		U high, low;
		position(static_cast<U>(pos), high, low);
		ChunkType mask = MASK >> low;
		return (m_chunks[high] & mask) != 0;
	}

	/// Any are enabled.
	Bool getAny() const
	{
		static const BitSet ZERO(false);
		return *this != ZERO;
	}

	/// Count bits.
	U getEnabledBitCount() const
	{
		U count = 0;
		for(U i = 0; i < CHUNK_COUNT; ++i)
		{
			count += __builtin_popcount(m_chunks[i]);
		}
		return count;
	}

protected:
	using ChunkType = TChunkType;

	/// Number of bits a chunk holds.
	static const U CHUNK_BIT_COUNT = sizeof(ChunkType) * 8;

	/// Number of chunks.
	static const U CHUNK_COUNT = (N + (CHUNK_BIT_COUNT - 1)) / CHUNK_BIT_COUNT;

	/// A mask for some stuff.
	static const ChunkType MASK = ChunkType(1) << (CHUNK_BIT_COUNT - 1);

	ChunkType m_chunks[CHUNK_COUNT];

	BitSet()
	{
	}

	static void position(U bit, U& high, U& low)
	{
		ANKI_ASSERT(bit < N);
		high = bit / CHUNK_BIT_COUNT;
		low = bit % CHUNK_BIT_COUNT;
		ANKI_ASSERT(high < CHUNK_COUNT);
		ANKI_ASSERT(low < CHUNK_BIT_COUNT);
	}
};
/// @}

} // end namespace anki
