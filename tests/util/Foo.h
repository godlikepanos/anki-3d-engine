#ifndef ANKI_TESTS_UTIL_FOO_H
#define ANKI_TESTS_UTIL_FOO_H

#include <iostream>

/// Struct for testing
struct Foo
{
	int x = 666;
	static int constructorCallCount;
	static int destructorCallCount;

	Foo()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		++constructorCallCount;
	}

	Foo(int x_)
		: x(x_)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		++constructorCallCount;
	}

	Foo(const Foo& b)
		: x(b.x)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		++constructorCallCount;
	}

	Foo(Foo&& b)
		: x(b.x)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		b.x = 0;
		++constructorCallCount;
	}

	~Foo()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		++destructorCallCount;
	}

	Foo& operator=(const Foo& b)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		x = b.x;
		return *this;
	}

	Foo& operator=(Foo&& b)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		x = b.x;
		b.x = 0;
		return *this;
	}

	static void reset()
	{
		destructorCallCount = constructorCallCount = 0;
	}
};

#endif
