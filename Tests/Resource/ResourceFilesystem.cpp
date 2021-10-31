// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Resource/ResourceFilesystem.h>

namespace anki {

ANKI_TEST(Resource, ResourceFilesystem)
{
	printf("Test requires the Data dir\n");

	HeapAllocator<U8> alloc(allocAligned, nullptr);
	ResourceFilesystem fs(alloc);

	{
		ANKI_TEST_EXPECT_NO_ERR(fs.addNewPath("Tests/Data/Dir/../Dir/", StringListAuto(alloc)));
		ResourceFilePtr file;
		ANKI_TEST_EXPECT_NO_ERR(fs.openFile("subdir0/hello.txt", file));
		StringAuto txt(alloc);
		ANKI_TEST_EXPECT_NO_ERR(file->readAllText(txt));
		ANKI_TEST_EXPECT_EQ(txt, "hello\n");
	}

	{
		ANKI_TEST_EXPECT_NO_ERR(fs.addNewPath("./Tests/Data/Dir.AnKiZLibip", StringListAuto(alloc)));
		ResourceFilePtr file;
		ANKI_TEST_EXPECT_NO_ERR(fs.openFile("subdir0/hello.txt", file));
		StringAuto txt(alloc);
		ANKI_TEST_EXPECT_NO_ERR(file->readAllText(txt));
		ANKI_TEST_EXPECT_EQ(txt, "hell\n");
	}
}

} // end namespace anki
