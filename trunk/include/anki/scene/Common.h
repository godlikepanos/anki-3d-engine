#ifndef ANKI_SCENE_COMMON_H
#define ANKI_SCENE_COMMON_H

#include "anki/util/Allocator.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// The type of the scene's allocator
template<typename T>
using SceneAllocator = StackAllocator<T, false>;

/// Scene string
typedef std::basic_string<char, std::char_traits<char>,
	SceneAllocator<char>> SceneString;

/// Scene vector
template<typename T>
using SceneVector = Vector<T, SceneAllocator<T>>;

} // end namespace anki

#endif
