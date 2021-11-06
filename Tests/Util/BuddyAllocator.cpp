// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/BuddyAllocator.h>

namespace anki {

ANKI_TEST(Util, BuddyAllocator)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Simple
	{
		BuddyAllocator<4> buddy;

		Array<U32, 2> addr;
		Bool success = buddy.allocate(alloc, 1, addr[0]);
		success = buddy.allocate(alloc, 3, addr[1]);

		// buddy.debugPrint();

		buddy.free(alloc, addr[0], 1);
		buddy.free(alloc, addr[1], 3);

		// printf("\n");
		// buddy.debugPrint();
	}

	// Fuzzy
	{
		BuddyAllocator<32> buddy;
		std::vector<std::pair<U32, U32>> allocations;
		for(U32 it = 0; it < 1000; ++it)
		{
			if((getRandom() % 2) == 0)
			{
				// Do an allocation
				U32 addr;
				const U32 size = max<U32>(getRandom() % 512, 1);
				const Bool success = buddy.allocate(alloc, size, addr);
				if(success)
				{
					allocations.push_back({addr, size});
				}
			}
			else
			{
				// Do some deallocation
				if(allocations.size())
				{
					const PtrSize randPos = getRandom() % allocations.size();
					buddy.free(alloc, allocations[randPos].first, allocations[randPos].second);

					allocations.erase(allocations.begin() + randPos);
				}
			}
		}

		// Remove the remaining
		for(const auto& pair : allocations)
		{
			buddy.free(alloc, pair.first, pair.second);
		}
	}
}

} // end namespace anki
