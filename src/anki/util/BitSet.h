// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Array.h>
#include <initializer_list>
#include <cstring>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// Easy bit manipulation.
/// @tparam N The number of bits.
/// @tparam TChunkType The type of the chunks that the bitset consists. By default it's U8.
template<U32 N, typename TChunkType = U8>
class BitSet
{
protected:
	using ChunkType = TChunkType;

	/// Number of bits a chunk holds.
	static constexpr U32 CHUNK_BIT_COUNT = sizeof(ChunkType) * 8;

	/// Number of chunks.
	static constexpr U32 CHUNK_COUNT = (N + (CHUNK_BIT_COUNT - 1)) / CHUNK_BIT_COUNT;

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
		for(U32 i = 0; i < CHUNK_COUNT; ++i)
		{
			out.m_chunks[i] = m_chunks[i] | b.m_chunks[i];
		}
		return out;
	}

	/// Bitwise or between this and @a b sets.
	BitSet& operator|=(const BitSet& b)
	{
		for(U32 i = 0; i < CHUNK_COUNT; ++i)
		{
			m_chunks[i] = m_chunks[i] | b.m_chunks[i];
		}
		return *this;
	}

	/// Bitwise and between this and @a b sets.
	BitSet operator&(const BitSet& b) const
	{
		BitSet out;
		for(U32 i = 0; i < CHUNK_COUNT; ++i)
		{
			out.m_chunks[i] = m_chunks[i] & b.m_chunks[i];
		}
		return out;
	}

	/// Bitwise and between this and @a b sets.
	BitSet& operator&=(const BitSet& b)
	{
		for(U32 i = 0; i < CHUNK_COUNT; ++i)
		{
			m_chunks[i] = m_chunks[i] & b.m_chunks[i];
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
		for(U32 i = 0; i < CHUNK_COUNT; ++i)
		{
			m_chunks[i] = m_chunks[i] ^ b.m_chunks[i];
		}
		return *this;
	}

	/// Bitwise not of self.
	BitSet operator~() const
	{
		BitSet out;
		for(U32 i = 0; i < CHUNK_COUNT; ++i)
		{
			out.m_chunks[i] = TChunkType(~m_chunks[i]);
		}
		out.zeroUnusedBits();
		return out;
	}

	Bool operator==(const BitSet& b) const
	{
		Bool same = m_chunks[0] == b.m_chunks[0];
		for(U32 i = 1; i < CHUNK_COUNT; ++i)
		{
			same = same && (m_chunks[i] == b.m_chunks[i]);
		}
		return same;
	}

	Bool operator!=(const BitSet& b) const
	{
		return !(*this == b);
	}

	Bool operator!() const
	{
		return !getAny();
	}

	explicit operator Bool() const
	{
		return getAny();
	}

	/// Set or unset a bit at the given position.
	template<typename TInt>
	void set(TInt pos, Bool setBits = true)
	{
		U32 high, low;
		position(U32(pos), high, low);
		const ChunkType mask = ChunkType(ChunkType(1) << ChunkType(low));
		m_chunks[high] = (setBits) ? ChunkType(m_chunks[high] | mask) : ChunkType(m_chunks[high] & ~mask);
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
		memset(&m_chunks[0], 0xFF, sizeof(m_chunks));
		zeroUnusedBits();
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
		memset(&m_chunks[0], 0, sizeof(m_chunks));
	}

	/// Flip the bits at the given position. It will go from 1 to 0 or from 0 to 1.
	template<typename TInt>
	void flip(TInt pos)
	{
		U32 high, low;
		position(U32(pos), high, low);
		const ChunkType mask = ChunkType(ChunkType(1) << ChunkType(low));
		m_chunks[high] ^= mask;
	}

	/// Return true if the bit is set or false if it's not.
	template<typename TInt>
	Bool get(TInt pos) const
	{
		U32 high, low;
		position(U32(pos), high, low);
		const ChunkType mask = ChunkType(ChunkType(1) << ChunkType(low));
		return (m_chunks[high] & mask) != 0;
	}

	/// Any are enabled.
	Bool getAny() const
	{
		static const BitSet ZERO(false);
		return *this != ZERO;
	}

	/// Count bits.
	U32 getEnabledBitCount() const
	{
		U32 count = 0;
		for(U i = 0; i < CHUNK_COUNT; ++i)
		{
			count += __builtin_popcount(m_chunks[i]);
		}
		return count;
	}

	/// Get the most significant bit that is enabled. Or MAX_U32 if all is zero.
	U32 getMostSignificantBit() const
	{
		U32 i = CHUNK_COUNT;
		while(i--)
		{
			const U64 bits = m_chunks[i];
			if(bits != 0)
			{
				const U32 msb = U32(__builtin_clzll(bits));
				return (63 - msb) + (i * CHUNK_BIT_COUNT);
			}
		}

		return MAX_U32;
	}

	Array<TChunkType, CHUNK_COUNT> getData() const
	{
		return m_chunks;
	}

protected:
	Array<ChunkType, CHUNK_COUNT> m_chunks;

	BitSet()
	{
	}

	static void position(U32 bit, U32& high, U32& low)
	{
		ANKI_ASSERT(bit < N);
		high = bit / CHUNK_BIT_COUNT;
		low = bit % CHUNK_BIT_COUNT;
		ANKI_ASSERT(high < CHUNK_COUNT);
		ANKI_ASSERT(low < CHUNK_BIT_COUNT);
	}

	/// Zero the unused bits.
	void zeroUnusedBits()
	{
		const ChunkType UNUSED_BITS = CHUNK_COUNT * CHUNK_BIT_COUNT - N;
		const ChunkType USED_BITMASK = std::numeric_limits<ChunkType>::max() >> UNUSED_BITS;
		if(USED_BITMASK > 0)
		{
			m_chunks[CHUNK_COUNT - 1] &= USED_BITMASK;
		}
	}
};
/// @}

} // end namespace anki
