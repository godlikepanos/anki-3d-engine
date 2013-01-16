#ifndef ANKI_TESTS_UTIL_FOO_H
#define ANKI_TESTS_UTIL_FOO_H

/// Struct for testing
struct Foo
{
	int x = 666;
	static int constructorCallCount;
	static int destructorCallCount;

	Foo()
	{
		++constructorCallCount;
	}

	Foo(int x_)
		: x(x_)
	{
		++constructorCallCount;
	}

	Foo(const Foo& b)
		: x(b.x)
	{
		++constructorCallCount;
	}

	Foo(Foo&& b)
		: x(b.x)
	{
		b.x = 0;
		++constructorCallCount;
	}

	~Foo()
	{
		++destructorCallCount;
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
