// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file It contains forward declarations of util classes.

#pragma once

#include <anki/util/StdTypes.h>

namespace anki
{

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

class String;
class StringAuto;

class ThreadHive;

} // end namespace anki
