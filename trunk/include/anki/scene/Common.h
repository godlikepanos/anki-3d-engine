#ifndef ANKI_SCENE_COMMON_H
#define ANKI_SCENE_COMMON_H

#include "anki/util/Allocator.h"

namespace anki {

/// The type of the scene's allocator
template<typename T>
using SceneAllocator = StackAllocator<T, false>;

} // end namespace anki

#endif
