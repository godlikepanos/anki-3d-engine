/// @file
/// Contains misc functions

#ifndef ANKI_UTIL_FUNCTIONS_H
#define ANKI_UTIL_FUNCTIONS_H

#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup misc
/// @{

/// Pick a random number from min to max
extern I32 randRange(I32 min, I32 max);

/// Pick a random number from min to max
extern U32 randRange(U32 min, U32 max);

/// Pick a random number from min to max
extern F32 randRange(F32 min, F32 max);

/// Pick a random number from min to max
extern F64 randRange(F64 min, F64 max);

extern F32 randFloat(F32 max);

/// Get the size in bytes of a vector
template<typename Vec>
extern PtrSize getVectorSizeInBytes(const Vec& v)
{
	return v.size() * sizeof(typename Vec::value_type);
}

/// @}
/// @}

} // end namespace anki

#endif
