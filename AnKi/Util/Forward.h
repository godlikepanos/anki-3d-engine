// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file It contains forward declarations of util classes.

#pragma once

#include <AnKi/Util/StdTypes.h>

namespace anki {

template<U32 N, typename TChunkType>
class BitSet;

template<typename T>
class BitMask;

template<typename, typename, typename>
class HashMap;

template<typename T>
class Hierarchy;

template<typename T>
class List;

template<typename T>
class ListAuto;

template<typename T, typename TIndex>
class SparseArray;

class CString;
class String;
class StringAuto;

class ThreadHive;

template<typename T, PtrSize T_PREALLOCATED_STORAGE = ANKI_SAFE_ALIGNMENT>
class Function;

template<typename T, typename TSize = U32>
class WeakArray;

template<typename T, typename TSize = U32>
class ConstWeakArray;

template<typename T, typename TSize = U32>
class DynamicArray;

template<typename T, typename TSize = U32>
class DynamicArrayAuto;

} // end namespace anki
