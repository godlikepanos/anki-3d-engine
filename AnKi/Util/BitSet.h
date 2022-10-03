// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Array.h>
#include <initializer_list>
#include <cstring>

namespace anki {

/// @addtogroup util_containers
/// @{

/// Easy bit manipulation.
/// @tparam kBitCount The number of bits.
/// @tparam TChunkType The type of the chunks that the bitset consists. By default it's U8.
template<U32 kBitCount, typename TChunkType = U8>
class BitSet
{
private:
	using ChunkType = TChunkType;

	/// Number of bits a chunk holds.
	static constexpr U32 kChunkBitCount = sizeof(ChunkType) * 8;

	/// Number of chunks.
	static constexpr U32 kChunkCount = (kBitCount + (kChunkBitCount - 1)) / kChunkBitCount;

public:
	/// Constructor. It will set all the bits or unset them.
	BitSet(Bool set)
	{
		ANKI_ASSERT(set == 0 || set == 1);
		if(!set)
		{
			unsetAll();
		}
		else
		{
			setAll();
		}
	}

	/// Copy.
	BitSet(const BitSet& b)
		: m_chunks(b.m_chunks)
	{
	}

	/// Copy.
	BitSet& operator=(const BitSet& b)
	{
		m_chunks = b.m_chunks;
		return *this;
	}

	/// Bitwise or between this and @a b sets.
	BitSet operator|(const BitSet& b) const
	{
		BitSet out;
		for(U32 i = 0; i < kChunkCount; ++i)
		{
			out.m_chunks[i] = m_chunks[i] | b.m_chunks[i];
		}
		return out;
	}

	/// Bitwise or between this and @a b sets.
	BitSet& operator|=(const BitSet& b)
	{
		for(U32 i = 0; i < kChunkCount; ++i)
		{
			m_chunks[i] = m_chunks[i] | b.m_chunks[i];
		}
		return *this;
	}

	/// Bitwise and between this and @a b sets.
	BitSet operator&(const BitSet& b) const
	{
		BitSet out;
		for(U32 i = 0; i < kChunkCount; ++i)
		{
			out.m_chunks[i] = m_chunks[i] & b.m_chunks[i];
		}
		return out;
	}

	/// Bitwise and between this and @a b sets.
	BitSet& operator&=(const BitSet& b)
	{
		for(U32 i = 0; i < kChunkCount; ++i)
		{
			m_chunks[i] = m_chunks[i] & b.m_chunks[i];
		}
		return *this;
	}

	/// Bitwise xor between this and @a b sets.
	BitSet operator^(const BitSet& b) const
	{
		BitSet out;
		for(U i = 0; i < kChunkCount; ++i)
		{
			out.m_chunks[i] = m_chunks[i] ^ b.m_chunks[i];
		}
		return out;
	}

	/// Bitwise xor between this and @a b sets.
	BitSet& operator^=(const BitSet& b)
	{
		for(U32 i = 0; i < kChunkCount; ++i)
		{
			m_chunks[i] = m_chunks[i] ^ b.m_chunks[i];
		}
		return *this;
	}

	/// Bitwise not of self.
	BitSet operator~() const
	{
		BitSet out;
		for(U32 i = 0; i < kChunkCount; ++i)
		{
			out.m_chunks[i] = TChunkType(~m_chunks[i]);
		}
		out.zeroUnusedBits();
		return out;
	}

	Bool operator==(const BitSet& b) const
	{
		Bool same = m_chunks[0] == b.m_chunks[0];
		for(U32 i = 1; i < kChunkCount; ++i)
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
	BitSet& set(TInt pos, Bool setBit = true)
	{
		U32 high, low;
		position(U32(pos), high, low);
		const ChunkType mask = ChunkType(ChunkType(1) << ChunkType(low));
		m_chunks[high] = (setBit) ? ChunkType(m_chunks[high] | mask) : ChunkType(m_chunks[high] & ~mask);
		return *this;
	}

	/// Set multiple bits.
	template<typename TInt>
	BitSet& set(std::initializer_list<TInt> list, Bool setBits = true)
	{
		for(auto it : list)
		{
			set(it, setBits);
		}
		return *this;
	}

	/// Set all bits.
	BitSet& setAll()
	{
		memset(&m_chunks[0], 0xFF, sizeof(m_chunks));
		zeroUnusedBits();
		return *this;
	}

	/// Unset a bit (set to zero) at the given position.
	template<typename TInt>
	BitSet& unset(TInt pos)
	{
		return set(pos, false);
	}

	/// Unset multiple bits.
	template<typename TInt>
	BitSet& unset(std::initializer_list<TInt> list)
	{
		return set(list, false);
	}

	/// Unset all bits.
	BitSet& unsetAll()
	{
		memset(&m_chunks[0], 0, sizeof(m_chunks));
		return *this;
	}

	/// Flip the bits at the given position. It will go from 1 to 0 or from 0 to 1.
	template<typename TInt>
	BitSet& flip(TInt pos)
	{
		U32 high, low;
		position(U32(pos), high, low);
		const ChunkType mask = ChunkType(ChunkType(1) << ChunkType(low));
		m_chunks[high] ^= mask;
		return *this;
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
		const BitSet kZero(false);
		return *this != kZero;
	}

	/// Count bits.
	U32 getEnabledBitCount() const
	{
		U32 count = 0;
		for(U i = 0; i < kChunkCount; ++i)
		{
			count += __builtin_popcountl(m_chunks[i]);
		}
		return count;
	}

	/// Get the most significant bit that is enabled. Or kMaxU32 if all is zero.
	U32 getMostSignificantBit() const
	{
		U32 i = kChunkCount;
		while(i--)
		{
			const U64 bits = m_chunks[i];
			if(bits != 0)
			{
				const U32 msb = U32(__builtin_clzll(bits));
				return (63 - msb) + (i * kChunkBitCount);
			}
		}

		return kMaxU32;
	}

	Array<TChunkType, kChunkCount> getData() const
	{
		return m_chunks;
	}

private:
	Array<ChunkType, kChunkCount> m_chunks;

	BitSet()
	{
	}

	static void position(U32 bit, U32& high, U32& low)
	{
		ANKI_ASSERT(bit < kBitCount);
		high = bit / kChunkBitCount;
		low = bit % kChunkBitCount;
		ANKI_ASSERT(high < kChunkCount);
		ANKI_ASSERT(low < kChunkBitCount);
	}

	/// Zero the unused bits.
	void zeroUnusedBits()
	{
		constexpr ChunkType kUnusedBits = kChunkCount * kChunkBitCount - kBitCount;
		constexpr ChunkType kUsedBitmask = std::numeric_limits<ChunkType>::max() >> kUnusedBits;
		if(kUsedBitmask > 0)
		{
			m_chunks[kChunkCount - 1] &= kUsedBitmask;
		}
	}
};
/// @}

} // end namespace anki
