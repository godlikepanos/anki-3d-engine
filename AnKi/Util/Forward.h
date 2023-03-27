// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file It contains forward declarations of util classes.

#pragma once

#include <AnKi/Util/StdTypes.h>

namespace anki {

template<U32 kBitCount, typename TChunkType>
class BitSet;

template<typename T>
class BitMask;

template<typename, typename, typename, typename>
class HashMap;

template<typename, typename, typename, typename, typename>
class HashMapRaii;

template<typename TKey>
class DefaultHasher;

class HashMapSparseArrayConfig;

template<typename T>
class Hierarchy;

template<typename>
class List;

template<typename, typename>
class ListRaii;

template<typename T, typename TConfig>
class SparseArray;

class CString;
class String;

template<typename>
class BaseStringRaii;

template<typename>
class BaseStringListRaii;

class ThreadHive;

template<typename, PtrSize kPreallocatedStorage = ANKI_SAFE_ALIGNMENT>
class Function;

template<typename, PtrSize>
class Array;

template<typename T, typename TSize = U32>
class WeakArray;

template<typename T, typename TSize = U32>
class ConstWeakArray;

template<typename T, typename TSize = U32>
class DynamicArray;

template<typename, typename, typename>
class DynamicArrayRaii;

class F16;

/// This macro defines typedefs for all the common containers that take a memory pool using a singleton memory pool
/// memory pool.
#define ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(submoduleName, singletonMemoryPool) \
	using submoduleName##MemPoolWrapper = SingletonMemoryPoolWrapper<singletonMemoryPool>; \
	using submoduleName##String = BaseStringRaii<submoduleName##MemPoolWrapper>; \
	template<typename T> \
	using submoduleName##DynamicArray = DynamicArrayRaii<T, U32, submoduleName##MemPoolWrapper>; \
	template<typename TKey, typename TValue> \
	using submoduleName##HashMap = \
		HashMapRaii<TKey, TValue, DefaultHasher<TKey>, HashMapSparseArrayConfig, submoduleName##MemPoolWrapper>; \
	template<typename T> \
	using submoduleName##List = ListRaii<T, submoduleName##MemPoolWrapper>; \
	using submoduleName##StringList = BaseStringListRaii<submoduleName##MemPoolWrapper>;

} // end namespace anki
