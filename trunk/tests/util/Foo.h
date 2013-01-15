#ifndef ANKI_TESTS_UTIL_FOO_H
#define ANKI_TESTS_UTIL_FOO_H

/// Struct for testing
struct Foo
{
	int x = 666;

	Foo()
	{}

	Foo(int x_)
		: x(x_)
	{}

	Foo(const Foo& b)
		: x(b.x)
	{}

	Foo(Foo&& b)
		: x(b.x)
	{
		b.x = 0;
	}

	Foo& operator=(const Foo& b)
	{
		x = b.x;
		return *this;
	}

	Foo& operator=(Foo&& b)
	{
		x = b.x;
		b.x = 0;
		return *this;
	}
};

#endif
