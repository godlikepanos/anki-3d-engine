// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_DICTIONARY_H
#define ANKI_UTIL_DICTIONARY_H

#include "anki/util/Allocator.h"
#include "anki/util/String.h"
#include <unordered_map>

namespace anki {

/// @addtogroup util_containers
/// @{

/// The hash function
class DictionaryHasher
{
public:
	PtrSize operator()(const CString& cstr) const
	{
		PtrSize h = 0;
		auto str = cstr.get();
		for (; *str != '\0'; ++str)
		{
			h += *str;
		}
		return h;
	}
};

/// The collision evaluation function
class DictionaryEqual
{
public:
	Bool operator()(const CString& a, const CString& b) const
	{
		return a == b;
	}
};

/// The hash map that has as key an old school C string. When inserting the
/// char MUST NOT point to a temporary or the evaluation function will fail.
/// Its template struct because C++ does not offer template typedefs
template<
	typename T, 
	template <typename> class TAlloc = HeapAllocator>
using Dictionary = 
	std::unordered_map<
		CString,
		T,
		DictionaryHasher,
		DictionaryEqual,
		TAlloc<std::pair<CString, T>>>;

/// @}

} // end namespace anki

#endif
