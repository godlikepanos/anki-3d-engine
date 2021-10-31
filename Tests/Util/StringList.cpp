// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/StringList.h>

namespace anki {

ANKI_TEST(Util, StringList)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Test splitString
	{
		CString toSplit = "foo\n\nboo\n";

		StringListAuto list(alloc);
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

} // end namespace anki
