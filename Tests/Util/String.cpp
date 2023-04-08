// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/String.h>
#include <string>

ANKI_TEST(Util, String)
{
	// Copy
	{
		String a, b;
		a = "123";
		b = a;

		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(b, "123");

		b.destroy();
		a.destroy();
		b = "321";
		a = b;
		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(a, "321");

		b.destroy();
		a.destroy();
	}

	// Move
	{
		String a;
		a = "123";
		String b(std::move(a));
		ANKI_TEST_EXPECT_EQ(a.isEmpty(), true);
		ANKI_TEST_EXPECT_EQ(b, "123");

		b.destroy();
		b = "321";
		a = std::move(b);
		ANKI_TEST_EXPECT_EQ(a, "321");
		ANKI_TEST_EXPECT_EQ(b.isEmpty(), true);
		a.destroy();
	}

	// Accessors
	{
		const char* s = "123";
		String a;
		a = s;
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

	// Append
	{
		String a, b;

		b = "123";

		a += b;
		ANKI_TEST_EXPECT_EQ(a, "123");

		a += "456789";
		a += String();
		a += "";
		a += "0";
		ANKI_TEST_EXPECT_EQ(a, "1234567890");
	}

	// Compare
	{
#define COMPARE(x_, y_, op_) \
	a += x_; \
	b += y_; \
	ANKI_TEST_EXPECT_EQ(a op_ b, std::string(x_) op_ std::string(y_)) \
	a.destroy(); \
	b.destroy();

		String a, b;
		COMPARE("123", "1233", <);
		COMPARE("0123", "1233", <=);
		COMPARE("ASDFA", "asdf90f", >);
		COMPARE(" %^*^^&", "aslkdfjb", >=);

#undef COMPARE
	}

	// sprintf
	{
		String a;

		// Simple
		a.sprintf("12%c  %d", '3', 123);
		ANKI_TEST_EXPECT_EQ(a, "123  123");
		a.destroy();

		// Extreme
		const char* s = "1234567890ABCDEF!@#$%^&*()_+asfghjkl:,.;ljk\"><{}[]/";
		a.sprintf("%s%s%s%s%s%s%s%s%s%s%s %d", s, s, s, s, s, s, s, s, s, s, s, 88);

		String b;
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
		String a;

		// Simple
		a.sprintf("12%c  %d", '3', 123);
		ANKI_TEST_EXPECT_EQ(a, "123  123");
		a.destroy();

		// Extreme
		const char* s = "12345";
		a.sprintf("%s%s %d", s, s, 88);

		String b;
		for(U i = 0; i < 2; i++)
		{
			b += s;
		}
		b += " 88";

		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(a.getLength(), b.getLength());
	}

	// Other create
	{
		String a;

		a = String('1', 3);
		ANKI_TEST_EXPECT_EQ(a, "111");
		ANKI_TEST_EXPECT_EQ(a.getLength(), 3);
	}

	// toString
	{
		String a;
		a.toString(123);
		ANKI_TEST_EXPECT_EQ(a, "123");

		a.toString(123.123);
		ANKI_TEST_EXPECT_EQ(a, "123.123000");
	}

	// To number
	{
		I64 i;
		String a;
		a = "123456789";
		ANKI_TEST_EXPECT_NO_ERR(a.toNumber(i));
		ANKI_TEST_EXPECT_EQ(i, 123456789);

		a = "-9223372036854775807";
		ANKI_TEST_EXPECT_NO_ERR(a.toNumber(i));
		ANKI_TEST_EXPECT_EQ(i, -9223372036854775807);

		F64 f;
		a += "123456789.145";
		ANKI_TEST_EXPECT_NO_ERR(a.toNumber(f));
		ANKI_TEST_EXPECT_EQ(f, 123456789.145);
	}

	// replaceAll
	{
		String a = "foo";
		a.replaceAll("foo", "bar");
		ANKI_TEST_EXPECT_EQ(a, "bar");

		a = "lafooha";
		a.replaceAll("foo", "bar");
		ANKI_TEST_EXPECT_EQ(a, "labarha");

		a = "jjhfalkakljla";
		a.replaceAll("a", "b");
		ANKI_TEST_EXPECT_EQ(a, "jjhfblkbkljlb");

		a = "%foo%ajlkadsf%foo%";
		a.replaceAll("%foo%", "");
		ANKI_TEST_EXPECT_EQ(a, "ajlkadsf");
	}
}
