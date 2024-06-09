// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Renderer/Utils/TileAllocator.h>

ANKI_TEST(Renderer, TileAllocator)
{
	RendererMemoryPool::allocateSingleton(allocAligned, nullptr);

	{
		StackMemoryPool pool;
		pool.init(allocAligned, nullptr, 1024);

		TileAllocator::ArrayOfLightUuids kickedOutUuids(&pool);

		TileAllocator talloc;
		talloc.init(8, 8, 3, true);

		Array<U32, 4> viewport;
		TileAllocatorResult2 res;

		const U64 lightUuid = 1000;
		Timestamp crntTimestamp = 1;

		constexpr U kSmallTile = 0;
		constexpr U kMedTile = 1;
		constexpr U kBigTile = 2;

		// Allocate 1 med
		res = talloc.allocate(crntTimestamp, lightUuid + 1, kMedTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded);

		// Allocate 3 big
		res = talloc.allocate(crntTimestamp, lightUuid + 2, kBigTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);
		res = talloc.allocate(crntTimestamp, lightUuid + 3, kBigTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);
		res = talloc.allocate(crntTimestamp, lightUuid + 4, kBigTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);

		// Fail to allocate 1 big
		res = talloc.allocate(crntTimestamp, lightUuid + 5, kBigTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationFailed);

		// Allocate 3 med
		res = talloc.allocate(crntTimestamp, lightUuid + 6, kMedTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);
		res = talloc.allocate(crntTimestamp, lightUuid + 7, kMedTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);
		res = talloc.allocate(crntTimestamp, lightUuid + 8, kMedTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);

		// Fail to allocate a small
		res = talloc.allocate(crntTimestamp, lightUuid + 9, kSmallTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationFailed);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);

		// New frame
		++crntTimestamp;

		// Allocate the same 3 big again
		res = talloc.allocate(crntTimestamp, lightUuid + 2, kBigTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded | TileAllocatorResult2::kTileCached);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);
		res = talloc.allocate(crntTimestamp, lightUuid + 3, kBigTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded | TileAllocatorResult2::kTileCached);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);
		res = talloc.allocate(crntTimestamp, lightUuid + 4, kBigTile, viewport, kickedOutUuids);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded | TileAllocatorResult2::kTileCached);
		ANKI_TEST_EXPECT_EQ(kickedOutUuids.getSize(), 0);

		// New frame
		++crntTimestamp;

		// Allocate 16 small
		TileAllocator::ArrayOfLightUuids allKicked(&pool);
		for(U i = 0; i < 16; ++i)
		{
			res = talloc.allocate(crntTimestamp, lightUuid + 10 + i, kSmallTile, viewport, kickedOutUuids);
			ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult2::kAllocationSucceded);

			for(U64 uuid : kickedOutUuids)
			{
				allKicked.emplaceBack(uuid);
			}
		}

		// Check those that are kicked
		ANKI_TEST_EXPECT_EQ(allKicked.getSize(), 4);

		for(U64 uuid = lightUuid + 1; uuid <= lightUuid + 9; ++uuid)
		{
			auto it = std::find(allKicked.getBegin(), allKicked.getEnd(), uuid);

			if(uuid == lightUuid + 5 || uuid == lightUuid + 9)
			{
				// Allocation failures, skip
			}
			else if(uuid >= lightUuid + 2 && uuid <= lightUuid + 4)
			{
				// It's the big tiles, shouldn't have been kicked
				ANKI_TEST_EXPECT_EQ(it, allKicked.getEnd());
			}
			else
			{
				// Tiles from the 1st frame, should have been kicked all
				ANKI_TEST_EXPECT_NEQ(it, allKicked.getEnd());
			}
		}
	}

	RendererMemoryPool::freeSingleton();
}
