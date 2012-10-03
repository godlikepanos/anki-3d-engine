/// @file
/// Contains misc functions

#ifndef ANKI_UTIL_FUNCTIONS_H
#define ANKI_UTIL_FUNCTIONS_H

#include "anki/util/StdTypes.h"
#include <string>

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

/// Trim a string
inline std::string trimString(std::string& str, const char* what = " ")
{
	std::string out = str;
	out.erase(0, out.find_first_not_of(what));
	out.erase(out.find_last_not_of(what) + 1);
	return out;
}

/// @}
/// @}

} // end namespace anki

#endif
