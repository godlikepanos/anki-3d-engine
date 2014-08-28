// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_STRING_LIST_H
#define ANKI_UTIL_STRING_LIST_H

#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/util/String.h"
#include <algorithm>

namespace anki {

/// @addtogroup util_containers
/// @{

/// Sort method for StringList
enum class StringListSort
{
	ASCENDING,
	DESCENDING
};

/// A simple convenience class for string lists
template<typename TChar, typename TAlloc>
class BasicStringList: public Vector<BasicString<TChar, TAlloc>, TAlloc>
{
public:
	using Self = BasicStringList; ///< Self type
	using Char = TChar; ///< Char type
	using Allocator = TAlloc;
	using String = BasicString<Char, Allocator>; ///< String type
	using Base = Vector<String, Allocator>; ///< Base

	// Use the base constructors
	using Base::Base;

	/// Join all the elements into a single big string using a the
	/// seperator @a separator
	String join(const Char* separator) const;

	/// Returns the index position of the last occurrence of @a value in
	/// the list
	/// @return -1 of not found
	I getIndexOf(const Char* value) const;

	/// Sort the string list
	void sortAll(const StringListSort method = StringListSort::ASCENDING)
	{
		if(method == StringListSort::ASCENDING)
		{
			std::sort(Base::begin(), Base::end(), compareStringsAsc);
		}
		else
		{
			ANKI_ASSERT(method == StringListSort::DESCENDING);
			std::sort(Base::begin(), Base::end(), compareStringsDesc);
		}
	}

	/// Split a string using a separator (@a separator) and return these
	/// strings in a string list
	static Self splitString(const Char* s, const Char separator,
		Allocator alloc = Allocator());

private:
	static Bool compareStringsAsc(const String& a, const String& b)
	{
		return a < b;
	}

	static Bool compareStringsDesc(const String& a,	const String& b)
	{
		return a > b;
	}
};

/// A common string list allocated in heap
using StringList = BasicStringList<char, HeapAllocator<char>>;

/// @}

} // end namespace anki

#include "anki/util/StringList.inl.h"

#endif
