// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file It contains forward declarations of util classes.

#pragma once

#include <AnKi/Util/StdTypes.h>

namespace anki {

class DefaultMemoryPool;

template<typename TMemoryPool>
class SingletonMemoryPoolWrapper;

template<U32 kBitCount, typename TChunkType>
class BitSet;

template<typename T>
class BitMask;

template<typename, typename, typename, typename, typename>
class HashMap;

template<typename TKey>
class DefaultHasher;

class HashMapSparseArrayConfig;

template<typename T, typename TMemoryPool>
class Hierarchy;

template<typename T, typename TMemoryPool>
class List;

template<typename T, typename TMemoryPool, typename TConfig>
class SparseArray;

class CString;

template<typename>
class BaseString;

template<typename>
class BaseStringList;

class ThreadHive;

template<typename TFunc, typename TMemoryPool = SingletonMemoryPoolWrapper<DefaultMemoryPool>,
		 PtrSize kPreallocatedStorage = ANKI_SAFE_ALIGNMENT>
class Function;

template<typename, PtrSize>
class Array;

template<typename T, typename TSize>
class WeakArray;

template<typename T, typename TSize>
class ConstWeakArray;

template<typename T, typename TMemoryPool, typename TSize>
class DynamicArray;

class F16;

template<typename TMemoryPool>
class XmlDocument;

/// This macro defines typedefs for all the common containers that take a memory pool using a singleton memory pool
/// memory pool.
#define ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(submoduleName, singletonMemoryPool) \
	using submoduleName##MemPoolWrapper = SingletonMemoryPoolWrapper<singletonMemoryPool>; \
	using submoduleName##String = BaseString<submoduleName##MemPoolWrapper>; \
	template<typename T, typename TSize = U32> \
	using submoduleName##DynamicArray = DynamicArray<T, submoduleName##MemPoolWrapper, TSize>; \
	template<typename TKey, typename TValue, typename THasher = DefaultHasher<TKey>> \
	using submoduleName##HashMap = \
		HashMap<TKey, TValue, THasher, submoduleName##MemPoolWrapper, HashMapSparseArrayConfig>; \
	template<typename T> \
	using submoduleName##List = List<T, submoduleName##MemPoolWrapper>; \
	using submoduleName##StringList = BaseStringList<submoduleName##MemPoolWrapper>; \
	using submoduleName##XmlDocument = XmlDocument<submoduleName##MemPoolWrapper>; \
	template<typename T> \
	using submoduleName##Hierarchy = Hierarchy<T, submoduleName##MemPoolWrapper>;

} // end namespace anki
