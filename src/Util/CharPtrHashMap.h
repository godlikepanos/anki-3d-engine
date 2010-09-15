#ifndef CHAR_PTR_HASH_MAP_H
#define CHAR_PTR_HASH_MAP_H

#include <boost/unordered_map.hpp>
#include <cstring>
#include "Common.h"


/**
 * The hash function
 */
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


/**
 * The collision evaluation function
 */
struct CompareCharPtrHashMapKeys
{
  bool operator()(const char* a, const char* b) const
  {
    return strcmp(a, b) == 0;
  }
};


/**
 * The hash map that has as key an old school C string. When inserting the char MUST NOT point to a temporary or the
 * evaluation function will fail.
 */
template<typename Type>
class CharPtrHashMap: public unordered_map<const char*, Type, CreateCharPtrHashMapKey, CompareCharPtrHashMapKeys>
{};


#endif
