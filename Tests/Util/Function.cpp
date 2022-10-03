// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Util/Foo.h>
#include <AnKi/Util/Function.h>

namespace anki {

static I32 functionAcceptingFunction(const Function<I32(F32)>& f)
{
	return f(1.0f) + f(2.0f);
}

} // end namespace anki

ANKI_TEST(Util, Function)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	// Simple
	{
		Function<I32(I32, F32), 8> f;

		I32 i = 0;
		f.init(pool, [&i](U32 a, F32 b) -> I32 {
			i += I32(a) + I32(b);
			return i;
		});

		i = 1;

		f(2, 10.0f);
		ANKI_TEST_EXPECT_EQ(i, 13);

		f.destroy(pool);
	}

	// No templates
	{
		F32 f = 1.1f;

		Function<I32(F32)> func(pool, [&f](F32 ff) {
			f += 2.0f;
			return I32(f) + I32(ff);
		});

		const I32 o = functionAcceptingFunction(func);
		func.destroy(pool);

		ANKI_TEST_EXPECT_EQ(o, 11);
	}

	// Allocated
	{
		const Vec4 a(1.9f);
		const Vec4 b(2.2f);
		Function<Vec4(Vec4, Vec4), 8> f(pool, [a, b](Vec4 c, Vec4 d) mutable -> Vec4 {
			return a + c * 2.0f + b * d;
		});

		const Vec4 r = f(Vec4(10.0f), Vec4(20.8f));
		ANKI_TEST_EXPECT_EQ(r, a + Vec4(10.0f) * 2.0f + b * Vec4(20.8f));

		f.destroy(pool);
	}

	// Complex
	{
		{
			Foo foo, bar;
			Function<void(Foo&)> f(pool, [foo](Foo& r) {
				r.x += foo.x;
			});

			Function<void(Foo&)> ff;
			ff = std::move(f);

			ff(bar);

			ANKI_TEST_EXPECT_EQ(bar.x, 666 * 2);
			ff.destroy(pool);
		}

		ANKI_TEST_EXPECT_EQ(Foo::constructorCallCount, Foo::destructorCallCount);
		Foo::reset();
	}

	// Copy
	{
		{
			Foo foo, bar;
			Function<void(Foo&)> f(pool, [foo](Foo& r) {
				r.x += foo.x;
			});

			Function<void(Foo&)> ff;
			ff.copy(f, pool);

			ff(bar);

			ANKI_TEST_EXPECT_EQ(bar.x, 666 * 2);
			ff.destroy(pool);
			f.destroy(pool);
		}

		ANKI_TEST_EXPECT_EQ(Foo::constructorCallCount, Foo::destructorCallCount);
		Foo::reset();
	}
}
