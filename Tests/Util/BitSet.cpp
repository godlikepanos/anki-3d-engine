// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/BitSet.h>
#include <bitset>

using namespace anki;

template<U32 N, typename T>
static void randomBitsets(BitSet<N, T>& bitset, std::bitset<N>& stlBitset)
{
	for(U32 bit = 0; bit < N; ++bit)
	{
		const Bool set = rand() & 1;
		bitset.set(bit, set);
		stlBitset[bit] = set;
	}
}

template<U32 N, typename T>
static void fuzzyTest()
{
	BitSet<N, T> bitset = {false};
	std::bitset<N> stlBitset;

	for(U32 i = 0; i < 10000; ++i)
	{
		const U32 bit = rand() % N;
		const U32 set = rand() & 1;

		bitset.set(bit, set);
		stlBitset[bit] = set;

		const U32 mode = rand() % 10;
		BitSet<N, T> otherBitset = {false};
		std::bitset<N> otherStlBitset;
		randomBitsets<N, T>(otherBitset, otherStlBitset);
		if(mode == 0)
		{
			stlBitset.flip();
			bitset = ~bitset;
		}
		else if(mode == 1)
		{
			bitset |= otherBitset;
			stlBitset |= otherStlBitset;
		}
		else if(mode == 2)
		{
			bitset = bitset | otherBitset;
			stlBitset = stlBitset | otherStlBitset;
		}
		else if(mode == 3)
		{
			bitset &= otherBitset;
			stlBitset &= otherStlBitset;
		}
		else if(mode == 4)
		{
			bitset = bitset & otherBitset;
			stlBitset = stlBitset & otherStlBitset;
		}
		else if(mode == 5)
		{
			bitset ^= otherBitset;
			stlBitset ^= otherStlBitset;
		}
		else if(mode == 6)
		{
			bitset = bitset ^ otherBitset;
			stlBitset = stlBitset ^ otherStlBitset;
		}
	}

	for(U32 bit = 0; bit < N; ++bit)
	{
		const Bool set = bitset.get(bit);
		const Bool stlSet = stlBitset[bit];
		ANKI_TEST_EXPECT_EQ(set, stlSet);
	}
}

template<typename TChunkType>
static void checkMsb()
{
	const U N = 256;
	BitSet<N, TChunkType> a = {false};

	ANKI_TEST_EXPECT_EQ(a.getMostSignificantBit(), kMaxU32);

	a.set(32);
	ANKI_TEST_EXPECT_EQ(a.getMostSignificantBit(), 32);

	a.set(33);
	ANKI_TEST_EXPECT_EQ(a.getMostSignificantBit(), 33);

	a.set(68);
	ANKI_TEST_EXPECT_EQ(a.getMostSignificantBit(), 68);

	a.unset(68);
	ANKI_TEST_EXPECT_EQ(a.getMostSignificantBit(), 33);

	a.unsetAll();
	a.set(255);
	ANKI_TEST_EXPECT_EQ(a.getMostSignificantBit(), 255);
}

template<typename TChunkType>
static void checkLsb()
{
	constexpr U32 kCount = 256;
	BitSet<kCount, TChunkType> a = {false};

	ANKI_TEST_EXPECT_EQ(a.getLeastSignificantBit(), kMaxU32);

	a.set(33);
	ANKI_TEST_EXPECT_EQ(a.getLeastSignificantBit(), 33);

	a.set(34);
	ANKI_TEST_EXPECT_EQ(a.getLeastSignificantBit(), 33);

	a.set(2);
	ANKI_TEST_EXPECT_EQ(a.getLeastSignificantBit(), 2);

	a.unset(2);
	ANKI_TEST_EXPECT_EQ(a.getLeastSignificantBit(), 33);

	a.setAll();
	ANKI_TEST_EXPECT_EQ(a.getLeastSignificantBit(), 0);
}

template<typename TChunkType>
static void checkSetNLeastSignificant()
{
	constexpr U32 kCount = 256;
	BitSet<kCount, TChunkType> a = {true};

	for(U32 n = 1; n <= kCount; ++n)
	{
		a.setAll();
		a.unsetNLeastSignificantBits(n);
		for(U i = 0; i < kCount; ++i)
		{
			ANKI_TEST_EXPECT_EQ(a.get(i), (i < n) ? false : true);
		}
	}
}

ANKI_TEST(Util, BitSet)
{
	{
#define ANKI_BITSET_TEST(N) \
	fuzzyTest<N, U8>(); \
	fuzzyTest<N, U16>(); \
	fuzzyTest<N, U32>(); \
	fuzzyTest<N, U64>();

		ANKI_BITSET_TEST(4)
		ANKI_BITSET_TEST(9)
		ANKI_BITSET_TEST(33)
		ANKI_BITSET_TEST(250)
		ANKI_BITSET_TEST(30)

#undef ANKI_BITSET_TEST
	}

	checkMsb<U8>();
	checkMsb<U16>();
	checkMsb<U32>();
	checkMsb<U64>();

	checkLsb<U8>();
	checkLsb<U16>();
	checkLsb<U32>();
	checkLsb<U64>();

	checkSetNLeastSignificant<U8>();
	checkSetNLeastSignificant<U16>();
	checkSetNLeastSignificant<U32>();
	checkSetNLeastSignificant<U64>();
}
