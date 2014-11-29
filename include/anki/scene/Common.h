// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_COMMON_H
#define ANKI_SCENE_COMMON_H

#include "anki/util/Allocator.h"
#include "anki/util/String.h"
#include "anki/util/Dictionary.h"

namespace anki {

// Forward
class SceneNode;

/// @addtogroup Scene
/// @{

/// The type of the scene's allocator
template<typename T>
using SceneAllocator = StackAllocator<T, false>;

/// The type of the scene's frame allocator
template<typename T>
using SceneFrameAllocator = StackAllocator<T, false>;

/// Scene string
using SceneString = StringBase<SceneAllocator<char>>;

/// Scene array
template<typename T>
using SceneDArray = DArray<T, SceneAllocator<T>>;

/// The same as SceneDArray. Different name to show the difference
template<typename T>
using SceneFrameDArray = DArray<T, SceneFrameAllocator<T>>;

/// Scene dictionary
template<typename T>
using SceneDictionary = 
	Dictionary<T, SceneAllocator<std::pair<const char*, T>>>;
/// @}

} // end namespace anki

#endif
