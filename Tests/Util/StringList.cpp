// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/StringList.h>

ANKI_TEST(Util, StringList)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	// Test splitString
	{
		CString toSplit = "foo\n\nboo\n";

		StringListRaii list(&pool);
		list.splitString(toSplit, '\n');

		ANKI_TEST_EXPECT_EQ(list.getSize(), 2);
		auto it = list.getBegin();
		ANKI_TEST_EXPECT_EQ(*it, "foo");
		++it;
		ANKI_TEST_EXPECT_EQ(*it, "boo");

		// Again
		list.destroy();
		list.splitString(toSplit, '\n', true);
		ANKI_TEST_EXPECT_EQ(list.getSize(), 3);
		it = list.getBegin();
		ANKI_TEST_EXPECT_EQ(*it, "foo");
		++it;
		ANKI_TEST_EXPECT_EQ(it->isEmpty(), true);
		++it;
		ANKI_TEST_EXPECT_EQ(*it, "boo");
	}
}
