// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/BuddyAllocatorBuilder.h>
#include <tuple>

using namespace anki;

/// Check if all memory has the same value.
static int memvcmp(const void* memory, U8 val, PtrSize size)
{
	const U8* mm = static_cast<const U8*>(memory);
	return (*mm == val) && memcmp(mm, mm + 1, size - 1) == 0;
}

ANKI_TEST(Util, BuddyAllocatorBuilder)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	// Simple
	{
		BuddyAllocatorBuilder<32, Mutex> buddy(&pool, 32);

		Array<U32, 2> addr;
		const Array<U32, 2> sizes = {58, 198010775};
		const Array<U32, 2> alignments = {21, 17};
		Bool success = buddy.allocate(sizes[0], alignments[0], addr[0]);
		ANKI_TEST_EXPECT_EQ(success, true);
		success = buddy.allocate(sizes[1], alignments[1], addr[1]);
		ANKI_TEST_EXPECT_EQ(success, true);

		// buddy.debugPrint();

		buddy.free(addr[0], sizes[0], alignments[0]);
		buddy.free(addr[1], sizes[1], alignments[1]);

		// printf("\n");
		// buddy.debugPrint();
	}

	// Fuzzy with alignment
	{
		BuddyAllocatorBuilder<32, Mutex> buddy(&pool, 32);
		std::vector<std::tuple<U32, U32, U32, U8>> allocations;

		U8* backingMemory = static_cast<U8*>(malloc(kMaxU32));
		for(PtrSize i = 0; i < kMaxU32; ++i)
		{
			backingMemory[i] = i % kMaxU8;
		}

		for(U32 it = 0; it < 10000; ++it)
		{
			if((getRandom() % 2) == 0)
			{
				// Do an allocation
				U32 addr;
				const U32 size = max<U32>(getRandom() % 256_MB, 1);
				const U32 alignment = max<U32>(getRandom() % 24, 1);
				const Bool success = buddy.allocate(size, alignment, addr);
				// printf("al %u %u\n", size, alignment);
				if(success)
				{
					const U8 bufferValue = getRandom() % kMaxU8;
					memset(backingMemory + addr, bufferValue, size);
					allocations.push_back({addr, size, alignment, bufferValue});
				}
			}
			else
			{
				// Do some deallocation
				if(allocations.size())
				{
					const PtrSize randPos = getRandom() % allocations.size();

					const U32 address = std::get<0>(allocations[randPos]);
					const U32 size = std::get<1>(allocations[randPos]);
					const U32 alignment = std::get<2>(allocations[randPos]);
					const U8 bufferValue = std::get<3>(allocations[randPos]);

					ANKI_TEST_EXPECT_EQ(memvcmp(backingMemory + address, bufferValue, size), 1);

					// printf("fr %u %u\n", size, alignment);
					buddy.free(address, size, alignment);

					allocations.erase(allocations.begin() + randPos);
				}
			}
		}
		free(backingMemory);

		// Get the fragmentation
		BuddyAllocatorBuilderStats stats;
		buddy.getStats(stats);
		ANKI_TEST_LOGI("Memory info: userAllocatedSize %zu, realAllocatedSize %zu, externalFragmentation %f, "
					   "internalFragmentation %f",
					   stats.m_userAllocatedSize, stats.m_realAllocatedSize, stats.m_externalFragmentation,
					   stats.m_internalFragmentation);

		// Remove the remaining
		for(const auto& pair : allocations)
		{
			buddy.free(std::get<0>(pair), std::get<1>(pair), std::get<2>(pair));
		}
	}
}
