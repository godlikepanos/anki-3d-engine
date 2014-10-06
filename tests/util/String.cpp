// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/util/String.h"
#include <string>

namespace anki {

//==============================================================================
ANKI_TEST(Util, String)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Copy
	{
		String a("123", alloc);
		String b(a);
		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(b, "123");

		b = "321";
		a = b;
		ANKI_TEST_EXPECT_EQ(a, "321");
	}

	// Move
	{
		String a("123", alloc);
		String b(std::move(a));
		ANKI_TEST_EXPECT_EQ(a.isEmpty(), true);
		ANKI_TEST_EXPECT_EQ(b, "123");

		b = "321";
		a = std::move(b);
		ANKI_TEST_EXPECT_EQ(a, "321");
		ANKI_TEST_EXPECT_EQ(b.isEmpty(), true);
	}

	// Accessors
	{
		const char* s = "123";
		String a(s, alloc);
		ANKI_TEST_EXPECT_EQ(a[0], '1');
		ANKI_TEST_EXPECT_EQ(a[1], '2');
		ANKI_TEST_EXPECT_EQ(a[2], '3');

		U count = 0;
		for(char& c : a)
		{
			++c;
			++count;
		}
	
		ANKI_TEST_EXPECT_EQ(a, "234");
		ANKI_TEST_EXPECT_EQ(count, 3);

		ANKI_TEST_EXPECT_EQ(a.begin(), &a[0]);
		ANKI_TEST_EXPECT_EQ(a.end(), &a[0] + 3);
	}

	// Addition
	{
		String a(alloc);
		String b(alloc);

		a = "123";
		b = String("456", alloc);

		String c;
		c = a + b;

		ANKI_TEST_EXPECT_EQ(c, "123456");
	}

	// Append
	{
		String a(alloc);
		String b("123", alloc);

		a += b;
		ANKI_TEST_EXPECT_EQ(a, "123");

		a += "456";
		a += String("789", alloc);
		a += String(alloc);
		a += "";
		a += "0";
		ANKI_TEST_EXPECT_EQ(a, "1234567890");
	}

	// Compare
	{
#define COMPARE(x_, y_, op_) \
	ANKI_TEST_EXPECT_EQ(String(x_, alloc) op_ String(y_, alloc), \
	std::string(x_) op_ std::string(y_) )

		COMPARE("123", "1233", <);
		COMPARE("0123", "1233", <=);
		COMPARE("ASDFA", "asdf90f", >);
		COMPARE(" %^*^^&", "aslkdfjb", >=);

#undef COMPARE
	}

	// sprintf
	{
		String a(alloc);

		// Simple
		a.sprintf("12%c  %d", '3', 123);
		ANKI_TEST_EXPECT_EQ(a, "123  123");

		// Extreme
		const char* s = "1234567890ABCDEF!@#$%^&*()_+asfghjkl:,.;ljk\"><{}[]/";
		a.sprintf("%s%s%s%s%s%s%s%s%s%s%s %d", 
			s, s, s, s, s, s, s, s, s, s, s, 88);

		String b(alloc);
		for(U i = 0; i < 11; i++)
		{
			b += s;
		}
		b += " 88";

		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(a.getLength(), b.getLength());
	}

	// sprintf #2: Smaller result (will trigger another path)
	{
		String a(alloc);

		// Simple
		a.sprintf("12%c  %d", '3', 123);
		ANKI_TEST_EXPECT_EQ(a, "123  123");

		// Extreme
		const char* s = "12345";
		a.sprintf("%s%s %d", s, s, 88);

		String b(alloc);
		for(U i = 0; i < 2; i++)
		{
			b += s;
		}
		b += " 88";

		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(a.getLength(), b.getLength());
	}

	// Resize
	{
		String a(alloc);

		a.resize(3, '1');
		ANKI_TEST_EXPECT_EQ(a, "111");

		a.resize(2, '2');
		ANKI_TEST_EXPECT_EQ(a, "11");

		a.resize(3, '3');
		ANKI_TEST_EXPECT_EQ(a, "113");

		a.resize(3, '?');
		ANKI_TEST_EXPECT_EQ(a, "113");

		a.resize(4, '4');
		ANKI_TEST_EXPECT_EQ(a, "1134");
	}

	// toString
	{
		String a(String::toString(123, alloc));
		ANKI_TEST_EXPECT_EQ(a, "123");

		a = String::toString(123.123, alloc);
		ANKI_TEST_EXPECT_EQ(a, "123.123000");
	}

	// To number
	{
		I64 i = String("123456789", alloc).toI64();
		ANKI_TEST_EXPECT_EQ(i, 123456789);

		i = String("-9223372036854775807", alloc).toI64();
		ANKI_TEST_EXPECT_EQ(i, -9223372036854775807);

		F64 f = String("123456789.145", alloc).toF64();
		ANKI_TEST_EXPECT_EQ(f, 123456789.145);
	}
}

} // end namespace anki

