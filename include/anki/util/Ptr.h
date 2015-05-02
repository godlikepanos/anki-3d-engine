// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_PTR_H
#define ANKI_UTIL_PTR_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util_other
/// @{

/// A simple template class to wrap simple pointers
template<typename T>
class Ptr
{
public:
	Ptr()
	:	m_ptr(nullptr)
	{}

	Ptr(T* ptr)
	:	m_ptr(ptr)
	{}

	Ptr(const Ptr& other)
	:	m_ptr(other.m_ptr)
	{}

	Ptr(Ptr&& other)
	:	m_ptr(other.m_ptr)
	{
		other.m_ptr = nullptr;
	}

	/// Copy
	Ptr& operator=(const Ptr& other)
	{
		m_ptr = other.m_ptr;
		return *this;
	}

	/// Move.
	Ptr& operator=(Ptr&& other)
	{
		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
		return *this;
	}

	/// Dereference
	T& operator*() const
	{
		ANKI_ASSERT(m_ptr);
		return *m_ptr;
	}

	/// Dereference
	T* operator->() const
	{
		ANKI_ASSERT(m_ptr);
		return m_ptr;
	}

	/// @name Compare operators
	/// @{
	operator bool() const 
	{ 
    	return m_ptr != nullptr; 
	}

	Bool operator==(const Ptr& other) const
	{
		return m_ptr == other.m_ptr;
	}

	Bool operator!=(const Ptr& other) const
	{
		return m_ptr != other.m_ptr;
	}

	Bool operator<(const Ptr& other) const
	{
		return m_ptr < other.m_ptr;
	}

	Bool operator<=(const Ptr& other) const
	{
		return m_ptr <= other.m_ptr;
	}

	Bool operator>(const Ptr& other) const
	{
		return m_ptr > other.m_ptr;
	}

	Bool operator>=(const Ptr& other) const
	{
		return m_ptr >= other.m_ptr;
	}
	/// @}

	/// @name Arithmetic operators
	/// @{
	Ptr& operator++()
	{
		ANKI_ASSERT(m_ptr);
		++m_ptr;
		return *this;
	}

	Ptr& operator--()
	{
		ANKI_ASSERT(m_ptr);
		--m_ptr;
		return *this;
	}

	Ptr operator+(const Ptr& other) const
	{
		ANKI_ASSERT(m_ptr);
		Ptr out(m_ptr + other.m_ptr);
		return out;
	}

	Ptr& operator+=(const Ptr& other)
	{
		ANKI_ASSERT(m_ptr);
		m_ptr += other.m_ptr;
		return *this;
	}

	Ptr operator-(const Ptr& other) const
	{
		ANKI_ASSERT(m_ptr);
		Ptr out(m_ptr - other.m_ptr);
		return out;
	}

	Ptr& operator-=(const Ptr& other)
	{
		ANKI_ASSERT(m_ptr);
		m_ptr -= other.m_ptr;
		return *this;
	}
	/// @}

private:
	T* m_ptr;
};

/// UniquePtr default deleter.
template<typename T>
class UniquePtrDeleter
{
public:
	void operator()(T* x)
	{
		auto alloc = x->getAllocator();
		alloc.template deleteInstance<T>(x);
	}
};

/// A unique pointer.
template<typename T, typename TDeleter = UniquePtrDeleter<T>>
class UniquePtr
{
public:
	UniquePtr()
	:	m_ptr(nullptr)
	{}

	explicit UniquePtr(T* ptr)
	:	m_ptr(ptr)
	{}

	/// Non-copyable.
	UniquePtr(const UniquePtr& other) = delete;

	/// Move.
	UniquePtr(UniquePtr&& other)
	:	m_ptr(nullptr)
	{
		move(other);
	}

	/// Non-copyable.
	UniquePtr& operator=(const UniquePtr& other) = delete;

	/// Move.
	UniquePtr& operator=(UniquePtr&& other)
	{
		move(other);
		return *this;
	}

	/// Dereference
	T& operator*() const
	{
		ANKI_ASSERT(m_ptr);
		return *m_ptr;
	}

	/// Dereference
	T* operator->() const
	{
		ANKI_ASSERT(m_ptr);
		return m_ptr;
	}

	/// Set a new pointer. Will destroy the previous.
	void reset(T* ptr)
	{
		destroy();
		m_ptr = ptr;
	}

	/// @name Compare operators
	/// @{
	operator bool() const 
	{ 
    	return m_ptr != nullptr; 
	}

	Bool operator==(const UniquePtr& other) const
	{
		return m_ptr == other.m_ptr;
	}

	Bool operator!=(const UniquePtr& other) const
	{
		return m_ptr != other.m_ptr;
	}

	Bool operator<(const UniquePtr& other) const
	{
		return m_ptr < other.m_ptr;
	}

	Bool operator<=(const UniquePtr& other) const
	{
		return m_ptr <= other.m_ptr;
	}

	Bool operator>(const UniquePtr& other) const
	{
		return m_ptr > other.m_ptr;
	}

	Bool operator>=(const UniquePtr& other) const
	{
		return m_ptr >= other.m_ptr;
	}
	/// @}

private:
	T* m_ptr;

	void destroy()
	{
		if(m_ptr)
		{
			TDeleter deleter;
			deleter(m_ptr);
			m_ptr = nullptr;
		}
	}

	void move(UniquePtr& b)
	{
		reset(b.m_ptr);
		b.m_ptr = nullptr;
	}
};
/// @}

} // end namespace anki

#endif

