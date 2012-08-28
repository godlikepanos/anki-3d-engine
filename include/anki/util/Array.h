#ifndef ANKI_UTIL_ARRAY_H
#define ANKI_UTIL_ARRAY_H

#include <array>

namespace anki {

/// @addtogroup util
/// @{
template<typename T, size_t size>
using Array = std::array<T, size>;
/// @}

} // end namespace anki

#endif
