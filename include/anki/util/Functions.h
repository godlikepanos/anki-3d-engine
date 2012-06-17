/// @file
/// Contains misc functions

#ifndef ANKI_UTIL_FUNCTIONS_H
#define ANKI_UTIL_FUNCTIONS_H

#include <cstdint>
#include <cstdlib> // For size_t

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup misc
/// @{

/// Pick a random number from min to max
extern int randRange(int min, int max);

/// Pick a random number from min to max
extern uint32_t randRange(uint32_t min, uint32_t max);

/// Pick a random number from min to max
extern float randRange(float min, float max);

/// Pick a random number from min to max
extern double randRange(double min, double max);

extern float randFloat(float max);

/// Get the size in bytes of a vector
template<typename Vec>
extern size_t getVectorSizeInBytes(const Vec& v)
{
	return v.size() * sizeof(typename Vec::value_type);
}

/// @}
/// @}

} // end namespace anki

#endif
