// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_ATOMIC_H
#define ANKI_UTIL_ATOMIC_H

#include "anki/util/StdTypes.h"
#include <atomic>

/// @addtogroup util_other
/// @{

// 32bit
using AtomicI32 = std::atomic<I32>;
using AtomicU32 = std::atomic<U32>;

// 64bit
using AtomicI64 = std::atomic<I64>;
using AtomicU64 = std::atomic<U64>;

/// @}

#endif

