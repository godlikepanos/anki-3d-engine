// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_STRING_LIST_H
#define ANKI_UTIL_STRING_LIST_H

#include "anki/util/String.h"
#include "anki/util/List.h"
#include <algorithm>

namespace anki {

/// @addtogroup util_containers
/// @{

/// A simple convenience class for string lists
class StringList: public List<String>
{
public:
	using Char = char; ///< Char type
	using Base = List<String>; ///< Base

	/// Sort method for sortAll().
	enum class Sort
	{
		ASCENDING,
		DESCENDING
	};

	// Use the base constructors
	using Base::Base;

	template<typename TAllocator>
	void destroy(TAllocator alloc);

	/// Join all the elements into a single big string using a the
	/// seperator @a separator
	template<typename TAllocator>
	void join(TAllocator alloc, const CString& separator, String& out) const;

	/// Returns the index position of the last occurrence of @a value in
	/// the list
	/// @return -1 of not found
	I getIndexOf(const CString& value) const;

	/// Sort the string list
	void sortAll(const Sort method = Sort::ASCENDING);

	/// Push at the end of the list a formated string
	template<typename TAllocator, typename... TArgs>
	void pushBackSprintf(TAllocator alloc, const TArgs&... args);

	/// Split a string using a separator (@a separator) and return these
	/// strings in a string list
	template<typename TAllocator>
	void splitString(TAllocator alloc, const CString& s, const Char separator);
};

/// String list with automatic destruction.
class StringListAuto: public StringList
{
public:
	using Base = StringList;

	/// Create using an allocator.
	template<typename TAllocator>
	StringListAuto(TAllocator alloc)
	:	Base(),
		m_alloc(alloc)
	{}

	/// Move.
	StringListAuto(StringListAuto&& b)
	:	Base()
	{
		move(b);
	}

	~StringListAuto()
	{
		Base::destroy(m_alloc);
	}

	/// Move.
	StringListAuto& operator=(StringListAuto&& b)
	{
		move(b);
		return *this;
	}

	/// Push at the end of the list a formated string
	template<typename... TArgs>
	void pushBackSprintf(const TArgs&... args)
	{
		Base::pushBackSprintf(m_alloc, args...);
	}

	/// Split a string using a separator (@a separator) and return these
	/// strings in a string list
	void splitString(const CString& s, const Base::Char separator)
	{
		Base::splitString(m_alloc, s, separator);
	}

private:
	GenericMemoryPoolAllocator<Char> m_alloc;

	void move(StringListAuto& b)
	{
		Base::operator=(std::move(b));
		m_alloc = std::move(b.m_alloc);
	}
};
/// @}

} // end namespace anki

#include "anki/util/StringList.inl.h"

#endif
