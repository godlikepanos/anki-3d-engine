// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/SegregatedListsAllocatorBuilder.h>
#include <AnKi/Util/HighRezTimer.h>
#include <Tests/Framework/Framework.h>

using namespace anki;

static constexpr U32 CLASS_COUNT = 6;

class SegregatedListsAllocatorBuilderChunk :
	public SegregatedListsAllocatorBuilderChunkBase<SegregatedListsAllocatorBuilderChunk, CLASS_COUNT>
{
};

class SegregatedListsAllocatorBuilderInterface
{
public:
	HeapAllocator<U8> m_alloc = {allocAligned, nullptr};
	static constexpr PtrSize m_chunkSize = 100_MB;

	static constexpr U32 getClassCount()
	{
		return CLASS_COUNT;
	}

	void getClassInfo(U32 idx, PtrSize& size) const
	{
		static const Array<PtrSize, getClassCount()> classes = {512_KB, 1_MB, 5_MB, 10_MB, 30_MB, m_chunkSize};
		size = classes[idx];
	}

	Error allocateChunk(SegregatedListsAllocatorBuilderChunk*& newChunk, PtrSize& chunkSize)
	{
		newChunk = m_alloc.newInstance<SegregatedListsAllocatorBuilderChunk>();
		chunkSize = m_chunkSize;
		return Error::kNone;
	}

	void deleteChunk(SegregatedListsAllocatorBuilderChunk* chunk)
	{
		m_alloc.deleteInstance(chunk);
	}

	static constexpr PtrSize getMinSizeAlignment()
	{
		return 4;
	}

	HeapAllocator<U8>& getAllocator()
	{
		return m_alloc;
	}

	HeapAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}
};

using SLAlloc = SegregatedListsAllocatorBuilder<SegregatedListsAllocatorBuilderChunk,
												SegregatedListsAllocatorBuilderInterface, DummyMutex>;

template<typename TAlloc>
static void printAllocatorBuilder(const TAlloc& sl)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	StringListRaii list(alloc);
	sl.printFreeBlocks(list);

	if(list.isEmpty())
	{
		return;
	}

	StringRaii str(alloc);
	list.join("", str);
	printf("%s\n", str.cstr());
}

template<Bool T_VALIDATE, U32 T_ITERATIONS, Bool T_STATS>
static void fuzzyTest()
{
	class Alloc
	{
	public:
		SegregatedListsAllocatorBuilderChunk* m_chunk;
		PtrSize m_address;
		PtrSize m_alignment;
		PtrSize m_size;
	};

	SLAlloc sl;
	std::vector<Alloc> allocs;
	allocs.reserve(T_ITERATIONS);

	const Second start = HighRezTimer::getCurrentTime();
	F64 avgFragmentation = 0.0f;
	F64 maxFragmetation = 0.0f;

	for(U32 i = 0; i < T_ITERATIONS; ++i)
	{
		const Bool doAllocation = (getRandom() % 2) == 0;
		if(doAllocation)
		{
			Alloc alloc;
			do
			{
				alloc.m_size = getRandom() % 70_MB;
				alloc.m_alignment = nextPowerOfTwo(getRandom() % 16);
			} while(alloc.m_size == 0 || alloc.m_alignment == 0);

			ANKI_TEST_EXPECT_NO_ERR(sl.allocate(alloc.m_size, alloc.m_alignment, alloc.m_chunk, alloc.m_address));

			allocs.push_back(alloc);
		}
		else if(allocs.size())
		{
			const U32 idx = U32(getRandom() % allocs.size());

			const Alloc alloc = allocs[idx];
			allocs.erase(allocs.begin() + idx);

			sl.free(alloc.m_chunk, alloc.m_address, alloc.m_size);
		}

		if(T_STATS)
		{
			const F64 f = sl.computeExternalFragmentation();
			avgFragmentation += f / F64(T_ITERATIONS);
			maxFragmetation = max(maxFragmetation, f);
		}

		// printAllocatorBuilder(sl);
		if(T_VALIDATE)
		{
			ANKI_TEST_EXPECT_NO_ERR(sl.validate());
		}
	}

	// Free the rest of the mem
	for(const Alloc& alloc : allocs)
	{
		sl.free(alloc.m_chunk, alloc.m_address, alloc.m_size);
	}

	if(T_STATS)
	{
		const Second end = HighRezTimer::getCurrentTime();
		const Second dt = end - start;
		ANKI_TEST_LOGI("Operations/sec %f. Avg external fragmentation %f. Max external fragmentation %f",
					   F64(T_ITERATIONS) / dt, avgFragmentation, maxFragmetation);
	}
}

ANKI_TEST(Util, SegregatedListsAllocatorBuilder)
{
	// Simple test
	{
		SLAlloc sl;

		SegregatedListsAllocatorBuilderChunk* chunk;
		PtrSize address;
		ANKI_TEST_EXPECT_NO_ERR(sl.allocate(66, 4, chunk, address));
		// printAllocatorBuilder(sl);
		ANKI_TEST_EXPECT_NO_ERR(sl.validate());

		SegregatedListsAllocatorBuilderChunk* chunk2;
		PtrSize address2;
		ANKI_TEST_EXPECT_NO_ERR(sl.allocate(512, 64, chunk2, address2));
		// printAllocatorBuilder(sl);
		ANKI_TEST_EXPECT_NO_ERR(sl.validate());

		SegregatedListsAllocatorBuilderChunk* chunk3;
		PtrSize address3;
		ANKI_TEST_EXPECT_NO_ERR(sl.allocate(4, 64, chunk3, address3));
		// printAllocatorBuilder(sl);
		ANKI_TEST_EXPECT_NO_ERR(sl.validate());

		sl.free(chunk, address2, 512);
		// printAllocatorBuilder(sl);
		ANKI_TEST_EXPECT_NO_ERR(sl.validate());

		sl.free(chunk, address3, 4);
		// printAllocatorBuilder(sl);
		ANKI_TEST_EXPECT_NO_ERR(sl.validate());

		sl.free(chunk, address, 66);
		ANKI_TEST_EXPECT_NO_ERR(sl.validate());
	}

	// Fuzzy test
	fuzzyTest<true, 1024, false>();
}

ANKI_TEST(Util, SegregatedListsAllocatorBuilderBenchmark)
{
	fuzzyTest<false, 2000000, true>();
}
