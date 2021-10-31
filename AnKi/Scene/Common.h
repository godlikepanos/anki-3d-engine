// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Allocator.h>
#include <AnKi/Util/String.h>
#include <AnKi/Scene/Forward.h>

namespace anki {

/// @addtogroup scene
/// @{

#define ANKI_SCENE_LOGI(...) ANKI_LOG("SCEN", NORMAL, __VA_ARGS__)
#define ANKI_SCENE_LOGE(...) ANKI_LOG("SCEN", ERROR, __VA_ARGS__)
#define ANKI_SCENE_LOGW(...) ANKI_LOG("SCEN", WARNING, __VA_ARGS__)
#define ANKI_SCENE_LOGF(...) ANKI_LOG("SCEN", FATAL, __VA_ARGS__)

/// The type of the scene's allocator
template<typename T>
using SceneAllocator = HeapAllocator<T>;

/// The type of the scene's frame allocator
template<typename T>
using SceneFrameAllocator = StackAllocator<T>;
/// @}

} // end namespace anki
