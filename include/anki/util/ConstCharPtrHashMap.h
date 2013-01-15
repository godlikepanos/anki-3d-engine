#ifndef ANKI_UTIL_CONST_CHAR_PTR_HASH_MAP_H
#define ANKI_UTIL_CONST_CHAR_PTR_HASH_MAP_H

#include "anki/util/Allocator.h"
#include <unordered_map>
#include <cstring>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup containers
/// @{

/// The hash function
struct CreateCharPtrHashMapKey
{
	size_t operator()(const char* str) const
	{
		size_t h = 0;
		for (; *str != '\0'; ++str)
		{
			h += *str;
		}
		return h;
	}
};

/// The collision evaluation function
struct CompareCharPtrHashMapKeys
{
	bool operator()(const char* a, const char* b) const
	{
		return strcmp(a, b) == 0;
	}
};

/// The hash map that has as key an old school C string. When inserting the
/// char MUST NOT point to a temporary or the evaluation function will fail.
/// Its template struct because C++ does not offer template typedefs
template<
	typename T,
	typename Alloc = Allocator<std::pair<const char*, T>>>
struct ConstCharPtrHashMap
{
	typedef std::unordered_map<
		const char*,
		T,
		CreateCharPtrHashMapKey,
		CompareCharPtrHashMapKeys,
		Alloc> Type;
};
/// @}
/// @}

} // end namespace anki

#endif
