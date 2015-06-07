// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/util/Filesystem.h"
#include "anki/util/File.h"

ANKI_TEST(Util, FileExists)
{
	// Create file
	File file;
	ANKI_TEST_EXPECT_NO_ERR(file.open("./tmp", File::OpenFlag::WRITE));
	file.close();

	// Check
	ANKI_TEST_EXPECT_EQ(fileExists("./tmp"), true);
}

ANKI_TEST(Util, Directory)
{
	// Destroy previous
	if(directoryExists("./dir"))
	{
		ANKI_TEST_EXPECT_NO_ERR(removeDirectory("./dir"));
	}

	// Create simple directory
	ANKI_TEST_EXPECT_NO_ERR(createDirectory("./dir"));
	File file;
	ANKI_TEST_EXPECT_NO_ERR(file.open("./dir/tmp", File::OpenFlag::WRITE));
	file.close();
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/tmp"), true);

	ANKI_TEST_EXPECT_NO_ERR(removeDirectory("./dir"));
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/tmp"), false);
	ANKI_TEST_EXPECT_EQ(directoryExists("./dir"), false);

	// A bit more complex
	ANKI_TEST_EXPECT_NO_ERR(createDirectory("./dir"));
	ANKI_TEST_EXPECT_NO_ERR(createDirectory("./dir/rid"));
	ANKI_TEST_EXPECT_NO_ERR(file.open("./dir/rid/tmp", File::OpenFlag::WRITE));
	file.close();
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/rid/tmp"), true);

	ANKI_TEST_EXPECT_NO_ERR(removeDirectory("./dir"));
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/rid/tmp"), false);
	ANKI_TEST_EXPECT_EQ(directoryExists("./dir/rid"), false);
	ANKI_TEST_EXPECT_EQ(directoryExists("./dir"), false);
}

ANKI_TEST(Util, HomeDir)
{
	HeapAllocator<char> alloc(allocAligned, nullptr);
	StringAuto out(alloc);

	ANKI_TEST_EXPECT_NO_ERR(getHomeDirectory(alloc, out));
	printf("home dir %s\n", &out[0]);
	ANKI_TEST_EXPECT_GT(out.getLength(), 0);
}

ANKI_TEST(Util, WalkDir)
{
	// Walk crnt dir
	U32 dirCount = 0;

	ANKI_TEST_EXPECT_NO_ERR(walkDirectoryTree(".", &dirCount,
		[](const CString& fname, void* pDirCount, Bool isDir) -> Error
		{
			if(isDir)
			{
				++(*static_cast<U32*>(pDirCount));
			}

			return ErrorCode::NONE;
		}));

	ANKI_TEST_EXPECT_GT(dirCount, 0);

	// Test error
	dirCount = 0;
	ANKI_TEST_EXPECT_ERR(walkDirectoryTree(".", &dirCount,
		[](const CString& fname, void* pDirCount, Bool isDir) -> Error
		{
			++(*static_cast<U32*>(pDirCount));
			return ErrorCode::FUNCTION_FAILED;
		}),
		ErrorCode::FUNCTION_FAILED);

	ANKI_TEST_EXPECT_EQ(dirCount, 1);
}
