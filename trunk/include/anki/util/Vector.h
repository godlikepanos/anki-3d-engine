#ifndef ANKI_UTIL_VECTOR_H
#define ANKI_UTIL_VECTOR_H

#include "anki/util/Assert.h"
#include "anki/util/Allocator.h"
#include <vector>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup containers
/// @{

template<typename T, typename Alloc = HeapAllocator<T>>
using Vector = std::vector<T, Alloc>;

/// @}
/// @}

} // end namespace anki

#endif
