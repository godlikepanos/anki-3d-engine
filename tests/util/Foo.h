// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <cstdio>

#ifndef ANKI_TESTS_FOO_VERBOSE
#	define ANKI_TESTS_FOO_VERBOSE 0
#endif

#if ANKI_TESTS_FOO_VERBOSE
#	define ANKI_TESTS_FOO_PRINT() printf("%s\n", __PRETTY_FUNCTION__)
#else
#	define ANKI_TESTS_FOO_PRINT() ((void)0)
#endif

/// Struct for testing
struct Foo
{
	int x = 666;
	static int constructorCallCount;
	static int destructorCallCount;

	Foo()
	{
		ANKI_TESTS_FOO_PRINT();
		++constructorCallCount;
	}

	Foo(int x_)
		: x(x_)
	{
		ANKI_TESTS_FOO_PRINT();
		++constructorCallCount;
	}

	Foo(const Foo& b)
		: x(b.x)
	{
		ANKI_TESTS_FOO_PRINT();
		++constructorCallCount;
	}

	Foo(Foo&& b)
		: x(b.x)
	{
		ANKI_TESTS_FOO_PRINT();
		b.x = 0;
		++constructorCallCount;
	}

	~Foo()
	{
		ANKI_TESTS_FOO_PRINT();
		++destructorCallCount;
	}

	Foo& operator=(const Foo& b)
	{
		ANKI_TESTS_FOO_PRINT();
		x = b.x;
		return *this;
	}

	Foo& operator=(Foo&& b)
	{
		ANKI_TESTS_FOO_PRINT();
		x = b.x;
		b.x = 0;
		return *this;
	}

	bool operator==(const Foo& b) const
	{
		return x == b.x;
	}

	bool operator!=(const Foo& b) const
	{
		return x != b.x;
	}

	static void reset()
	{
		destructorCallCount = constructorCallCount = 0;
	}
};
