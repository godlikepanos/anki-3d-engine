// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/resource/ResourceFilesystem.h"

namespace anki
{

ANKI_TEST(Resource, ResourceFilesystem)
{
	printf("Test requires the data dir\n");

	HeapAllocator<U8> alloc(allocAligned, nullptr);
	ResourceFilesystem fs(alloc);

	{
		ANKI_TEST_EXPECT_NO_ERR(fs.addNewPath("data/dir/../dir/"));
		ResourceFilePtr file;
		ANKI_TEST_EXPECT_NO_ERR(fs.openFile("subdir0/hello.txt", file));
		StringAuto txt(alloc);
		ANKI_TEST_EXPECT_NO_ERR(file->readAllText(txt));
		ANKI_TEST_EXPECT_EQ(txt, "hello\n");
	}

	{
		ANKI_TEST_EXPECT_NO_ERR(fs.addNewPath("./data/dir.ankizip"));
		ResourceFilePtr file;
		ANKI_TEST_EXPECT_NO_ERR(fs.openFile("subdir0/hello.txt", file));
		StringAuto txt(alloc);
		ANKI_TEST_EXPECT_NO_ERR(file->readAllText(txt));
		ANKI_TEST_EXPECT_EQ(txt, "hell\n");
	}
}

} // end namespace anki
