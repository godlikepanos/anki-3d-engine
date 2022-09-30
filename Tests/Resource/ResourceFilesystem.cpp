// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Resource/ResourceFilesystem.h>

ANKI_TEST(Resource, ResourceFilesystem)
{
	printf("Test requires the Data dir\n");

	ConfigSet config(allocAligned, nullptr);

	HeapMemoryPool pool(allocAligned, nullptr);
	ResourceFilesystem fs;
	ANKI_TEST_EXPECT_NO_ERR(fs.init(config, allocAligned, nullptr));

	{
		ANKI_TEST_EXPECT_NO_ERR(fs.addNewPath("Tests/Data/Dir/../Dir/", StringListRaii(&pool)));
		ResourceFilePtr file;
		ANKI_TEST_EXPECT_NO_ERR(fs.openFile("subdir0/hello.txt", file));
		StringRaii txt(&pool);
		ANKI_TEST_EXPECT_NO_ERR(file->readAllText(txt));
		ANKI_TEST_EXPECT_EQ(txt, "hello\n");
	}

	{
		ANKI_TEST_EXPECT_NO_ERR(fs.addNewPath("./Tests/Data/Dir.AnKiZLibip", StringListRaii(&pool)));
		ResourceFilePtr file;
		ANKI_TEST_EXPECT_NO_ERR(fs.openFile("subdir0/hello.txt", file));
		StringRaii txt(&pool);
		ANKI_TEST_EXPECT_NO_ERR(file->readAllText(txt));
		ANKI_TEST_EXPECT_EQ(txt, "hell\n");
	}
}
