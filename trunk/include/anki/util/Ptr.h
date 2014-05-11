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
	/// @name Constructors/Destructor
	/// @{
	Ptr()
#if ANKI_ASSERTIONS == 1
		: m_ptr(nullptr)
#endif
	{}

	Ptr(T* ptr)
		: m_ptr(ptr)
	{}

	Ptr(const Ptr& other)
		: m_ptr(other.m_ptr)
	{}
	/// @}

	/// Copy
	Ptr& operator=(const Ptr& other)
	{
		m_ptr = other.m_ptr;
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

/// @}

} // end namespace anki

#endif

