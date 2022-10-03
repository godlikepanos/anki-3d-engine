// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Renderer/TileAllocator.h>

ANKI_TEST(Renderer, TileAllocator)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	TileAllocator talloc;
	talloc.init(&pool, 8, 8, 3, true);

	Array<U32, 4> viewport;
	TileAllocatorResult res;

	const U lightUuid = 1;
	const U dcCount = 666;
	Timestamp crntTimestamp = 1;
	Timestamp lightTimestamp = 1;

	// Allocate 1 med
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 1, 0, dcCount, 1, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);

	// Allocate 3 big
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 2, 0, dcCount, 2, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 3, 0, dcCount, 2, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 4, 0, dcCount, 2, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);

	// Fail to allocate 1 big
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 5, 0, dcCount, 2, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationFailed);

	// Allocate 3 med
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 1, 1, dcCount, 1, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 1, 2, dcCount, 1, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 1, 3, dcCount, 1, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);

	// Fail to allocate a small
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 6, 0, dcCount, 0, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationFailed);

	// New frame
	++crntTimestamp;

	// Allocate 3 big again
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 2, 0, dcCount + 1, 2, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 3, 0, dcCount, 2, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kCached);
	res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 4, 0, dcCount + 1, 2, viewport);
	ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);

	// Allocate 16 small
	for(U i = 0; i < 16; ++i)
	{
		res = talloc.allocate(crntTimestamp, lightTimestamp, lightUuid + 6 + i, 0, dcCount, 0, viewport);
		ANKI_TEST_EXPECT_EQ(res, TileAllocatorResult::kAllocationSucceded);
	}
}
