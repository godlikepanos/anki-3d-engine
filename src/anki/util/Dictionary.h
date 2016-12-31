// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/String.h>
#include <unordered_map>

namespace anki
{

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
		for(; *str != '\0'; ++str)
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

/// The hash map that has as key an old school C string. When inserting the char MUST NOT point to a temporary or the
/// evaluation function will fail. Its template struct because C++ does not offer template typedefs
template<typename T, typename TAlloc = HeapAllocator<std::pair<CString, T>>>
using Dictionary = std::unordered_map<CString, T, DictionaryHasher, DictionaryEqual, TAlloc>;
/// @}

} // end namespace anki
