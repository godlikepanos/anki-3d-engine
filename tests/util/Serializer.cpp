// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/Serializer.h>
#include <tests/util/SerializerTest.h>

ANKI_TEST(Util, BinarySerializer)
{
	Array<ClassB, 2> b = {};

	b[0].m_array[0] = 2;
	b[0].m_array[1] = 3;
	b[0].m_array[2] = 4;
	Array<U32, 3> bDarr = {{0xFF12EE34, 0xAA12BB34, 0xCC12DD34}};
	b[0].m_darray = bDarr;

	b[1].m_array[0] = 255;
	b[1].m_array[1] = 127;
	b[1].m_array[2] = 55;
	Array<U32, 1> bDarr2 = {{0x12345678}};
	b[1].m_darray = bDarr2;

	ClassA a = {};
	a.m_array[0] = 123;
	a.m_array[1] = 56;
	a.m_u32 = 321;
	a.m_u64 = 0x123456789ABCDEFF;
	a.m_darray = b;

	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Serialize
	{
		File file;
		ANKI_TEST_EXPECT_NO_ERR(file.open("serialized.bin", FileOpenFlag::WRITE | FileOpenFlag::BINARY));
		BinarySerializer serializer;

		ANKI_TEST_EXPECT_NO_ERR(serializer.serialize(a, alloc, file));
	}

	// Deserialize
	{
		File file;
		ANKI_TEST_EXPECT_NO_ERR(file.open("serialized.bin", FileOpenFlag::READ | FileOpenFlag::BINARY));

		BinaryDeserializer deserializer;
		ClassA* pa;
		ANKI_TEST_EXPECT_NO_ERR(deserializer.deserialize(pa, alloc, file));

		ANKI_TEST_EXPECT_EQ(pa->m_array[0], a.m_array[0]);
		ANKI_TEST_EXPECT_EQ(pa->m_u32, a.m_u32);
		ANKI_TEST_EXPECT_EQ(pa->m_darray.getSize(), a.m_darray.getSize());

		for(U32 i = 0; i < pa->m_darray.getSize(); ++i)
		{
			ANKI_TEST_EXPECT_EQ(pa->m_darray[i].m_array[1], b[i].m_array[1]);

			for(U32 j = 0; j < pa->m_darray[i].m_darray.getSize(); ++j)
			{
				ANKI_TEST_EXPECT_EQ(pa->m_darray[i].m_darray[j], b[i].m_darray[j]);
			}
		}

		alloc.deleteInstance(pa);
	}
}
