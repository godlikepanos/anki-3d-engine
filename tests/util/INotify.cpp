// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/INotify.h>
#include <anki/util/Filesystem.h>

ANKI_TEST(Util, INotify)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Monitor a dir
	{
		CString dir = "in_test_dir";

		ANKI_TEST_EXPECT_NO_ERR(createDirectory(dir));

		{
			INotify in;
			ANKI_TEST_EXPECT_NO_ERR(in.init(alloc, dir));

			Bool modified;
			ANKI_TEST_EXPECT_NO_ERR(in.pollEvents(modified));
			ANKI_TEST_EXPECT_EQ(modified, false);

			File file;
			ANKI_TEST_EXPECT_NO_ERR(
				file.open(StringAuto(alloc).sprintf("%s/file.txt", dir.cstr()).toCString(), FileOpenFlag::WRITE));
			file.close();

			ANKI_TEST_EXPECT_NO_ERR(in.pollEvents(modified));
			ANKI_TEST_EXPECT_EQ(modified, true);
		}

		ANKI_TEST_EXPECT_NO_ERR(removeDirectory(dir, alloc));
	}
}
