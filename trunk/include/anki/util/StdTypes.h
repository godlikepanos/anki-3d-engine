// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_STD_TYPES_H
#define ANKI_UTIL_STD_TYPES_H

#include "anki/Config.h"
#include <cstdint>
#include <cstddef>
#include <limits>

namespace anki {

/// @addtogroup util_other
/// @{

typedef int8_t I8; ///< Integer 8bit
const I8 MAX_I8 = std::numeric_limits<I8>::max();
const I8 MIN_I8 = std::numeric_limits<I8>::min();

typedef int16_t I16; ///< Integer 16bit
const I16 MAX_I16 = std::numeric_limits<I16>::max();
const I16 MIN_I16 = std::numeric_limits<I16>::min();

typedef int32_t I32; ///< Integer 32bit
const I32 MAX_I32 = std::numeric_limits<I32>::max();
const I32 MIN_I32 = std::numeric_limits<I32>::min();

typedef int64_t I64; ///< Integer 64bit
const I64 MAX_I64 = std::numeric_limits<I64>::max();
const I64 MIN_I64 = std::numeric_limits<I64>::min();

typedef int_fast32_t I; ///< Fast signed integer at least 32bit
const I MAX_I = std::numeric_limits<I>::max();
const I MIN_I = std::numeric_limits<I>::min();

typedef uint8_t U8; ///< Unsigned integer 8bit
const U8 MAX_U8 = std::numeric_limits<U8>::max();
const U8 MIN_U8 = std::numeric_limits<U8>::min();

typedef uint16_t U16; ///< Unsigned integer 16bit
const U16 MAX_U16 = std::numeric_limits<U16>::max();
const U16 MIN_U16 = std::numeric_limits<U16>::min();

typedef uint32_t U32; ///< Unsigned integer 32bit
const U32 MAX_U32 = std::numeric_limits<U32>::max();
const U32 MIN_U32 = std::numeric_limits<U32>::min();

typedef uint64_t U64; ///< Unsigned integer 64bit
const U64 MAX_U64 = std::numeric_limits<U64>::max();
const U64 MIN_U64 = std::numeric_limits<U64>::min();

typedef uint_fast32_t U; ///< Fast unsigned integer at least 32bit
const U MAX_U = std::numeric_limits<U>::max();
const U MIN_U = std::numeric_limits<U>::min();

typedef size_t PtrSize; ///< Like size_t
const PtrSize MAX_PTR_SIZE = std::numeric_limits<PtrSize>::max();
const PtrSize MIN_PTR_SIZE = std::numeric_limits<PtrSize>::min();

typedef float F32; ///< Floating point 32bit
const F32 MAX_F32 = std::numeric_limits<F32>::max();
const F32 MIN_F32 = -std::numeric_limits<F32>::max();

typedef double F64; ///< Floating point 64bit
const F64 MAX_F64 = std::numeric_limits<F64>::max();
const F64 MIN_F64 = -std::numeric_limits<F64>::max();

typedef bool Bool; ///< Fast boolean type
typedef U8 Bool8; ///< Small 8bit boolean type

/// @}

} // end namespace anki

#endif
