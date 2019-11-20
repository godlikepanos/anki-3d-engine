// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/Serializer.h>
#include <tests/util/SerializerTest.h>

ANKI_TEST(Util, BinarySerializer)
{
	File file;
	ANKI_TEST_EXPECT_NO_ERR(file.open("serialized.bin", FileOpenFlag::WRITE | FileOpenFlag::BINARY));
	BinarySerializer serializer;

	ClassB b;
	b.m_array[0] = 2;
	b.m_array[1] = 3;
	b.m_array[2] = 4;
	Array<U32, 3> bDarr = {{0xFF12EE34, 0xAA12BB34, 0xCC12DD34}};
	b.m_darraySize = bDarr.getSize();
	b.m_darray = &bDarr[0];

	ClassA a;
	a.m_array[0] = 123;
	a.m_array[1] = 56;
	a.m_u32 = 1;
	a.m_u64 = 0x123456789ABCDEFF;
	a.m_darray = &b;

	ANKI_TEST_EXPECT_NO_ERR(serializer.serialize(a, file));
}
