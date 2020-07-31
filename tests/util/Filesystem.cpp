// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Destroy previous
	if(directoryExists("./dir"))
	{
		ANKI_TEST_EXPECT_NO_ERR(removeDirectory("./dir", alloc));
	}

	// Create simple directory
	ANKI_TEST_EXPECT_NO_ERR(createDirectory("./dir"));
	File file;
	ANKI_TEST_EXPECT_NO_ERR(file.open("./dir/tmp", FileOpenFlag::WRITE));
	file.close();
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/tmp"), true);

	ANKI_TEST_EXPECT_NO_ERR(removeDirectory("./dir", alloc));
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/tmp"), false);
	ANKI_TEST_EXPECT_EQ(directoryExists("./dir"), false);

	// A bit more complex
	ANKI_TEST_EXPECT_NO_ERR(createDirectory("./dir"));
	ANKI_TEST_EXPECT_NO_ERR(createDirectory("./dir/rid"));
	ANKI_TEST_EXPECT_NO_ERR(file.open("./dir/rid/tmp", FileOpenFlag::WRITE));
	file.close();
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/rid/tmp"), true);

	ANKI_TEST_EXPECT_NO_ERR(removeDirectory("./dir", alloc));
	return;
	ANKI_TEST_EXPECT_EQ(fileExists("./dir/rid/tmp"), false);
	ANKI_TEST_EXPECT_EQ(directoryExists("./dir/rid"), false);
	ANKI_TEST_EXPECT_EQ(directoryExists("./dir"), false);
}

ANKI_TEST(Util, HomeDir)
{
	HeapAllocator<char> alloc(allocAligned, nullptr);
	StringAuto out(alloc);

	ANKI_TEST_EXPECT_NO_ERR(getHomeDirectory(out));
	printf("home dir %s\n", &out[0]);
	ANKI_TEST_EXPECT_GT(out.getLength(), 0);
}

ANKI_TEST(Util, WalkDir)
{
	HeapAllocator<char> alloc(allocAligned, nullptr);

	class Path
	{
	public:
		CString m_path;
		Bool m_isDir;
	};

	class Ctx
	{
	public:
		Array<Path, 4> m_paths = {
			{{"./data", true}, {"./data/dir", true}, {"./data/file1", false}, {"./data/dir/file2", false}}};
		U32 m_foundMask = 0;
		HeapAllocator<char> m_alloc;
	} ctx;
	ctx.m_alloc = alloc;

	Error err = removeDirectory("./data", alloc);
	(void)err;

	// Create some dirs and some files
	for(U32 i = 0; i < ctx.m_paths.getSize(); ++i)
	{
		if(ctx.m_paths[i].m_isDir)
		{
			ANKI_TEST_EXPECT_NO_ERR(createDirectory(ctx.m_paths[i].m_path));
		}
		else
		{
			File file;
			ANKI_TEST_EXPECT_NO_ERR(file.open(ctx.m_paths[i].m_path, FileOpenFlag::WRITE));
		}
	}

	// Walk crnt dir
	ANKI_TEST_EXPECT_NO_ERR(walkDirectoryTree("./data", &ctx, [](const CString& fname, void* ud, Bool isDir) -> Error {
		Ctx& ctx = *static_cast<Ctx*>(ud);
		for(U32 i = 0; i < ctx.m_paths.getSize(); ++i)
		{
			StringAuto p(ctx.m_alloc);
			p.sprintf("./data/%s", fname.cstr());
			if(ctx.m_paths[i].m_path == p)
			{
				ANKI_TEST_EXPECT_EQ(ctx.m_paths[i].m_isDir, isDir);
				ctx.m_foundMask |= 1 << i;
			}
		}

		return Error::NONE;
	}));

	ANKI_TEST_EXPECT_EQ(ctx.m_foundMask, 0b1110);

	// Test error
	U32 count = 0;
	ANKI_TEST_EXPECT_ERR(walkDirectoryTree("./data///dir////", &count,
										   [](const CString& fname, void* pCount, Bool isDir) -> Error {
											   ++(*static_cast<U32*>(pCount));
											   return Error::FUNCTION_FAILED;
										   }),
						 Error::FUNCTION_FAILED);

	ANKI_TEST_EXPECT_EQ(count, 1);
}
