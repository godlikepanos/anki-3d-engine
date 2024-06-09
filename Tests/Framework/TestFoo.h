// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <Tests/Framework/Framework.h>

namespace anki {

/// A simple class used for testing containers.
class TestFoo
{
public:
	int m_x;

	static inline int m_constructorCount = 0;
	static inline int m_destructorCount = 0;
	static inline int m_copyCount = 0;
	static inline int m_moveCount = 0;

	TestFoo()
		: m_x(0)
	{
		++m_constructorCount;
	}

	TestFoo(int x)
		: m_x(x)
	{
		++m_constructorCount;
	}

	TestFoo(const TestFoo& b)
		: m_x(b.m_x)
	{
		++m_constructorCount;
	}

	TestFoo(TestFoo&& b)
		: m_x(b.m_x)
	{
		b.m_x = 0;
		++m_constructorCount;
	}

	~TestFoo()
	{
		++m_destructorCount;
	}

	TestFoo& operator=(const TestFoo& b)
	{
		m_x = b.m_x;
		++m_copyCount;
		return *this;
	}

	TestFoo& operator=(TestFoo&& b)
	{
		m_x = b.m_x;
		b.m_x = 0;
		++m_moveCount;
		return *this;
	}

	static void reset()
	{
		m_constructorCount = 0;
		m_destructorCount = 0;
		m_copyCount = 0;
		m_moveCount = 0;
	}
};

} // end namespace anki
