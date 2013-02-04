#ifndef ANKI_UTIL_STD_TYPES_H
#define ANKI_UTIL_STD_TYPES_H

#include <cstdint>
#include <cstddef>

namespace anki {

/// @addtogroup util
/// @{
typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef int_fast32_t I; ///< Fast signed integer at least 32bit

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef uint_fast32_t U; ///< fast unsigned integer at least 32bit

typedef size_t PtrSize; ///< Like size_t

typedef float F32;
typedef double F64;

typedef bool Bool;
typedef U8 Bool8;
/// @}

} // end namespace anki

#endif
