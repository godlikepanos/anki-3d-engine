// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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
	ANKI_TEST_EXPECT_NO_ERR(file.open("./tmp", FileOpenFlag::WRITE));
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
	ANKI_TEST_EXPECT_NO_ERR(file.open("./dir/tmp", FileOpenFlag::WRITE));
	file.close();
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/tmp"), true);

	ANKI_TEST_EXPECT_NO_ERR(removeDirectory("./dir"));
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/tmp"), false);
	ANKI_TEST_EXPECT_EQ(directoryExists("./dir"), false);

	// A bit more complex
	ANKI_TEST_EXPECT_NO_ERR(createDirectory("./dir"));
	ANKI_TEST_EXPECT_NO_ERR(createDirectory("./dir/rid"));
	ANKI_TEST_EXPECT_NO_ERR(file.open("./dir/rid/tmp", FileOpenFlag::WRITE));
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
	printf("Test requires the data dir\n");

	// Walk crnt dir
	U32 count = 0;
	ANKI_TEST_EXPECT_NO_ERR(
		walkDirectoryTree("./data/dir", &count, [](const CString& fname, void* pCount, Bool isDir) -> Error {
			if(isDir)
			{
				printf("-- %s\n", &fname[0]);
				++(*static_cast<U32*>(pCount));
			}

			return Error::NONE;
		}));

	ANKI_TEST_EXPECT_EQ(count, 3);

	// Walk again
	count = 0;
	ANKI_TEST_EXPECT_NO_ERR(
		walkDirectoryTree("./data/dir///////", &count, [](const CString& fname, void* pCount, Bool isDir) -> Error {
			printf("-- %s\n", &fname[0]);
			++(*static_cast<U32*>(pCount));

			return Error::NONE;
		}));

	ANKI_TEST_EXPECT_EQ(count, 6);

	// Test error
	count = 0;
	ANKI_TEST_EXPECT_ERR(walkDirectoryTree("./data///dir////",
							 &count,
							 [](const CString& fname, void* pCount, Bool isDir) -> Error {
								 printf("-- %s\n", &fname[0]);
								 ++(*static_cast<U32*>(pCount));
								 return Error::FUNCTION_FAILED;
							 }),
		Error::FUNCTION_FAILED);

	ANKI_TEST_EXPECT_EQ(count, 1);
}
