// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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

} // end namespace anki
