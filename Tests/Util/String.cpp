// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/String.h>
#include <string>

ANKI_TEST(Util, String)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	// Copy
	{
		String a, b;
		a.create(pool, "123");
		b.create(pool, a);

		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(b, "123");

		b.destroy(pool);
		a.destroy(pool);
		b.create(pool, "321");
		a.create(pool, b);
		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(a, "321");

		b.destroy(pool);
		a.destroy(pool);
	}

	// Move
	{
		String a;
		a.create(pool, "123");
		String b(std::move(a));
		ANKI_TEST_EXPECT_EQ(a.isEmpty(), true);
		ANKI_TEST_EXPECT_EQ(b, "123");

		b.destroy(pool);
		b.create(pool, "321");
		a = std::move(b);
		ANKI_TEST_EXPECT_EQ(a, "321");
		ANKI_TEST_EXPECT_EQ(b.isEmpty(), true);
		a.destroy(pool);
	}

	// Accessors
	{
		const char* s = "123";
		String a;
		a.create(pool, s);
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
		a.destroy(pool);
	}

	// Append
	{
		String a, b;

		b.create(pool, "123");

		a.append(pool, b);
		ANKI_TEST_EXPECT_EQ(a, "123");

		a.append(pool, "456789");
		a.append(pool, String());
		a.append(pool, "");
		a.append(pool, "0");
		ANKI_TEST_EXPECT_EQ(a, "1234567890");
		a.destroy(pool);
		b.destroy(pool);
	}

	// Compare
	{
#define COMPARE(x_, y_, op_) \
	a.append(pool, x_); \
	b.append(pool, y_); \
	ANKI_TEST_EXPECT_EQ(a op_ b, std::string(x_) op_ std::string(y_)) \
	a.destroy(pool); \
	b.destroy(pool);

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
		a.sprintf(pool, "12%c  %d", '3', 123);
		ANKI_TEST_EXPECT_EQ(a, "123  123");
		a.destroy(pool);

		// Extreme
		const char* s = "1234567890ABCDEF!@#$%^&*()_+asfghjkl:,.;ljk\"><{}[]/";
		a.sprintf(pool, "%s%s%s%s%s%s%s%s%s%s%s %d", s, s, s, s, s, s, s, s, s, s, s, 88);

		String b;
		for(U i = 0; i < 11; i++)
		{
			b.append(pool, s);
		}
		b.append(pool, " 88");

		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(a.getLength(), b.getLength());

		a.destroy(pool);
		b.destroy(pool);
	}

	// sprintf #2: Smaller result (will trigger another path)
	{
		String a;

		// Simple
		a.sprintf(pool, "12%c  %d", '3', 123);
		ANKI_TEST_EXPECT_EQ(a, "123  123");
		a.destroy(pool);

		// Extreme
		const char* s = "12345";
		a.sprintf(pool, "%s%s %d", s, s, 88);

		String b;
		for(U i = 0; i < 2; i++)
		{
			b.append(pool, s);
		}
		b.append(pool, " 88");

		ANKI_TEST_EXPECT_EQ(a, b);
		ANKI_TEST_EXPECT_EQ(a.getLength(), b.getLength());

		a.destroy(pool);
		b.destroy(pool);
	}

	// Other create
	{
		String a;

		a.create(pool, '1', 3);
		ANKI_TEST_EXPECT_EQ(a, "111");
		ANKI_TEST_EXPECT_EQ(a.getLength(), 3);

		a.destroy(pool);
	}

	// toString
	{
		String a;
		a.toString(pool, 123);
		ANKI_TEST_EXPECT_EQ(a, "123");
		a.destroy(pool);

		a.toString(pool, 123.123);
		ANKI_TEST_EXPECT_EQ(a, "123.123000");
		a.destroy(pool);
	}

	// To number
	{
		I64 i;
		String a;
		a.create(pool, "123456789");
		ANKI_TEST_EXPECT_NO_ERR(a.toNumber(i));
		ANKI_TEST_EXPECT_EQ(i, 123456789);
		a.destroy(pool);

		a.create(pool, "-9223372036854775807");
		ANKI_TEST_EXPECT_NO_ERR(a.toNumber(i));
		ANKI_TEST_EXPECT_EQ(i, -9223372036854775807);
		a.destroy(pool);

		F64 f;
		a.create(pool, "123456789.145");
		ANKI_TEST_EXPECT_NO_ERR(a.toNumber(f));
		ANKI_TEST_EXPECT_EQ(f, 123456789.145);
		a.destroy(pool);
	}

	// replaceAll
	{
		String a = {pool, "foo"};
		a.replaceAll(pool, "foo", "bar");
		ANKI_TEST_EXPECT_EQ(a, "bar");
		a.destroy(pool);

		a.create(pool, "lafooha");
		a.replaceAll(pool, "foo", "bar");
		ANKI_TEST_EXPECT_EQ(a, "labarha");
		a.destroy(pool);

		a.create(pool, "jjhfalkakljla");
		a.replaceAll(pool, "a", "b");
		ANKI_TEST_EXPECT_EQ(a, "jjhfblkbkljlb");
		a.destroy(pool);

		a.create(pool, "%foo%ajlkadsf%foo%");
		a.replaceAll(pool, "%foo%", "");
		ANKI_TEST_EXPECT_EQ(a, "ajlkadsf");
		a.destroy(pool);
	}
}
