// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_STRING_LIST_H
#define ANKI_UTIL_STRING_LIST_H

#include "anki/util/String.h"
#include <algorithm>

namespace anki {

/// @addtogroup util_containers
/// @{

/// A simple convenience class for string lists
template<typename TAlloc>
class StringListBase: public Vector<BasicString<TAlloc>, TAlloc>
{
public:
	using Self = StringListBase; ///< Self type
	using Char = char; ///< Char type
	using Allocator = TAlloc;
	using String = BasicString<TAlloc>; ///< String type
	using Base = Vector<String, Allocator>; ///< Base

	/// Sort method for sortAll().
	enum class Sort
	{
		ASCENDING,
		DESCENDING
	};

	// Use the base constructors
	using Base::Base;

	/// Join all the elements into a single big string using a the
	/// seperator @a separator
	String join(const CString& separator) const;

	/// Returns the index position of the last occurrence of @a value in
	/// the list
	/// @return -1 of not found
	I getIndexOf(const CString& value) const;

	/// Sort the string list
	void sortAll(const Sort method = Sort::ASCENDING)
	{
		if(method == Sort::ASCENDING)
		{
			std::sort(Base::begin(), Base::end(), compareStringsAsc);
		}
		else
		{
			ANKI_ASSERT(method == Sort::DESCENDING);
			std::sort(Base::begin(), Base::end(), compareStringsDesc);
		}
	}

	/// Split a string using a separator (@a separator) and return these
	/// strings in a string list
	static Self splitString(const CString& s, const Char separator,
		Allocator alloc);

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

/// A common string list allocated in heap.
using StringList = StringListBase<HeapAllocator<char>>;

/// @}

} // end namespace anki

#include "anki/util/StringList.inl.h"

#endif
