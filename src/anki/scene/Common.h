// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/String.h>
#include <anki/util/Dictionary.h>

namespace anki
{

// Forward
class SceneGraph;
class SceneNode;
class MoveComponent;
class DecalComponent;

/// @addtogroup Scene
/// @{

/// The type of the scene's allocator
template<typename T>
using SceneAllocator = ChainAllocator<T>;

/// The type of the scene's frame allocator
template<typename T>
using SceneFrameAllocator = StackAllocator<T>;

/// Scene dictionary
template<typename T>
using SceneDictionary = Dictionary<T, SceneAllocator<std::pair<const char*, T>>>;
/// @}

} // end namespace anki
