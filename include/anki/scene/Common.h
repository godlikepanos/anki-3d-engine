// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
using SceneAllocator = ChainAllocator<T>;

/// The type of the scene's frame allocator
template<typename T>
using SceneFrameAllocator = StackAllocator<T>;

/// Scene string
using SceneString = StringBase<SceneAllocator<char>>;

/// Scene array
template<typename T>
using SceneDArray = DArray<T>;

template<typename T>
using SceneFrameDArrayAuto = DArrayAuto<T, SceneFrameAllocator<T>>;

/// Scene dictionary
template<typename T>
using SceneDictionary = 
	Dictionary<T, SceneAllocator<std::pair<const char*, T>>>;
/// @}

} // end namespace anki

#endif
