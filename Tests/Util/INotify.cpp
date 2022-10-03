// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/INotify.h>
#include <AnKi/Util/Filesystem.h>

ANKI_TEST(Util, INotify)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	// Monitor a dir
	{
		CString dir = "in_test_dir";

		ANKI_TEST_EXPECT_NO_ERR(createDirectory(dir));

		{
			INotify in;
			ANKI_TEST_EXPECT_NO_ERR(in.init(&pool, dir));

			Bool modified;
			ANKI_TEST_EXPECT_NO_ERR(in.pollEvents(modified));
			ANKI_TEST_EXPECT_EQ(modified, false);

			File file;
			ANKI_TEST_EXPECT_NO_ERR(
				file.open(StringRaii(&pool).sprintf("%s/file.txt", dir.cstr()).toCString(), FileOpenFlag::kWrite));
			file.close();

			ANKI_TEST_EXPECT_NO_ERR(in.pollEvents(modified));
			ANKI_TEST_EXPECT_EQ(modified, true);
		}

		ANKI_TEST_EXPECT_NO_ERR(removeDirectory(dir, pool));
	}
}
