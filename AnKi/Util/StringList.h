// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
class StringList : public List<String>
{
public:
	using Char = char; ///< Char type
	using Base = List<String>; ///< Base

	/// Sort method for sortAll().
	enum class Sort
	{
		kAscending,
		kDescending
	};

	// Use the base constructors
	using Base::Base;

	explicit operator Bool() const
	{
		return !Base::isEmpty();
	}

	template<typename TMemPool>
	void destroy(TMemPool& pool);

	/// Join all the elements into a single big string using a the seperator @a separator
	template<typename TMemPool>
	void join(TMemPool& pool, const CString& separator, String& out) const
	{
		BaseStringRaii<TMemPool> outl(pool);
		join(separator, outl);
		out = std::move(outl);
	}

	/// Join all the elements into a single big string using a the seperator @a separator
	template<typename TMemPool>
	void join(const CString& separator, BaseStringRaii<TMemPool>& out) const;

	/// Returns the index position of the last occurrence of @a value in the list.
	/// @return -1 of not found
	I getIndexOf(const CString& value) const;

	/// Sort the string list
	void sortAll(const Sort method = Sort::kAscending);

	/// Push at the end of the list a formated string.
	template<typename TMemPool>
	ANKI_CHECK_FORMAT(2, 3)
	void pushBackSprintf(TMemPool& pool, const Char* fmt, ...);

	/// Push at the beginning of the list a formated string.
	template<typename TMemPool>
	ANKI_CHECK_FORMAT(2, 3)
	void pushFrontSprintf(TMemPool& pool, const Char* fmt, ...);

	/// Push back plain CString.
	template<typename TMemPool>
	void pushBack(TMemPool& pool, CString cstr)
	{
		String str;
		str.create(pool, cstr);

		Base::emplaceBack(pool);
		Base::getBack() = std::move(str);
	}

	/// Push front plain CString
	template<typename TMemPool>
	void pushFront(TMemPool& pool, CString cstr)
	{
		String str;
		str.create(pool, cstr);

		Base::emplaceFront(pool);
		Base::getFront() = std::move(str);
	}

	/// Split a string using a separator (@a separator) and return these strings in a string list.
	template<typename TMemPool>
	void splitString(TMemPool& pool, const CString& s, const Char separator, Bool keepEmpty = false);
};

/// String list with automatic destruction.
template<typename TMemPool = MemoryPoolPtrWrapper<BaseMemoryPool>>
class BaseStringListRaii : public StringList
{
public:
	using Base = StringList;
	using MemoryPool = TMemPool;

	/// Create using a mem pool.
	BaseStringListRaii(const MemoryPool& pool)
		: Base()
		, m_pool(pool)
	{
	}

	/// Move.
	BaseStringListRaii(BaseStringListRaii&& b)
		: Base()
	{
		move(b);
	}

	~BaseStringListRaii()
	{
		Base::destroy(m_pool);
	}

	/// Move.
	BaseStringListRaii& operator=(BaseStringListRaii&& b)
	{
		move(b);
		return *this;
	}

	/// Destroy.
	void destroy()
	{
		Base::destroy(m_pool);
	}

	/// Push at the end of the list a formated string
	ANKI_CHECK_FORMAT(1, 2)
	void pushBackSprintf(const Char* fmt, ...);

	/// Push at the beginning of the list a formated string
	ANKI_CHECK_FORMAT(1, 2)
	void pushFrontSprintf(const Char* fmt, ...);

	/// Push back plain CString.
	void pushBack(CString cstr)
	{
		Base::pushBack(m_pool, cstr);
	}

	/// Push front plain CString.
	void pushFront(CString cstr)
	{
		Base::pushFront(m_pool, cstr);
	}

	/// Pop front element.
	void popFront()
	{
		getFront().destroy(m_pool);
		Base::popFront(m_pool);
	}

	/// Split a string using a separator (@a separator) and return these strings in a string list.
	void splitString(const CString& s, const Base::Char separator, Bool keepEmpty = false)
	{
		Base::splitString(m_pool, s, separator, keepEmpty);
	}

private:
	MemoryPool m_pool;

	void move(BaseStringListRaii& b)
	{
		Base::operator=(std::move(b));
		m_pool = std::move(b.m_pool);
	}
};

using StringListRaii = BaseStringListRaii<>;
/// @}

} // end namespace anki

#include <AnKi/Util/StringList.inl.h>
