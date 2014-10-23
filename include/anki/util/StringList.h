// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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
template<typename TAlloc>
class StringListBase: public List<StringBase<TAlloc>, TAlloc>
{
public:
	using Self = StringListBase; ///< Self type
	using Char = char; ///< Char type
	using Allocator = TAlloc;
	using String = StringBase<Allocator>; ///< String type
	using Base = List<String, Allocator>; ///< Base

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
	ANKI_USE_RESULT Error join(
		Allocator alloc, const CString& separator, String& out) const;

	/// Returns the index position of the last occurrence of @a value in
	/// the list
	/// @return -1 of not found
	I getIndexOf(const CString& value) const;

	/// Sort the string list
	void sortAll(const Sort method = Sort::ASCENDING);

	/// Split a string using a separator (@a separator) and return these
	/// strings in a string list
	static ANKI_USE_RESULT Error splitString(
		Allocator alloc, const CString& s, const Char separator, Self& out);
};

/// A common string list allocated in heap.
using StringList = StringListBase<HeapAllocator<char>>;

/// @}

} // end namespace anki

#include "anki/util/StringList.inl.h"

#endif
