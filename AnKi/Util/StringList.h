// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/String.h>
#include <AnKi/Util/List.h>
#include <algorithm>
#include <cstdarg>

namespace anki {

/// @addtogroup util_containers
/// @{

/// A simple convenience class for string lists
template<typename TMemoryPool>
class BaseStringList : public List<BaseString<TMemoryPool>, TMemoryPool>
{
public:
	using Char = char; ///< Char type
	using StringType = BaseString<TMemoryPool>;
	using Base = List<StringType, TMemoryPool>; ///< Base
	using MemoryPool = TMemoryPool;

	/// Sort method for sortAll().
	enum class Sort
	{
		kAscending,
		kDescending
	};

	BaseStringList(const TMemoryPool& pool = TMemoryPool())
		: Base(pool)
	{
	}

	BaseStringList(BaseStringList&& b)
		: Base(std::move(b))
	{
	}

	BaseStringList(const BaseStringList& b)
		: Base(b)
	{
	}

	BaseStringList& operator=(const BaseStringList& b)
	{
		static_cast<Base&>(*this) = static_cast<Base&>(b);
		return *this;
	}

	BaseStringList& operator=(BaseStringList&& b)
	{
		static_cast<Base&>(*this) = std::move(static_cast<Base&>(b));
		return *this;
	}

	explicit operator Bool() const
	{
		return !Base::isEmpty();
	}

	/// Join all the elements into a single big string using a the seperator @a separator
	template<typename TStringMemoryPool>
	void join(const CString& separator, BaseString<TStringMemoryPool>& out) const;

	/// Returns the index position of the last occurrence of @a value in the list.
	/// @return -1 of not found
	I getIndexOf(const CString& value) const;

	/// Sort the string list
	void sortAll(const Sort method = Sort::kAscending);

	/// Push at the end of the list a formated string.
	ANKI_CHECK_FORMAT(1, 2)
	void pushBackSprintf(const Char* fmt, ...);

	/// Push at the beginning of the list a formated string.
	ANKI_CHECK_FORMAT(1, 2)
	void pushFrontSprintf(const Char* fmt, ...);

	/// Push back plain CString.
	typename Base::Iterator pushBack(CString cstr)
	{
		return Base::emplaceBack(cstr, Base::getMemoryPool());
	}

	/// Push front plain CString
	typename Base::Iterator pushFront(CString cstr)
	{
		return Base::emplaceFront(cstr, Base::getMemoryPool());
	}

	/// Split a string using a separator (@a separator) and return these strings in a string list.
	void splitString(const CString& s, const Char separator, Bool keepEmpty = false);
};

using StringList = BaseStringList<SingletonMemoryPoolWrapper<DefaultMemoryPool>>;
/// @}

} // end namespace anki

#include <AnKi/Util/StringList.inl.h>
