#ifndef ANKI_SCENE_COMMON_H
#define ANKI_SCENE_COMMON_H

#include "anki/util/Allocator.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup config 
/// @{
/// @addtogroup config_scene Scene configuration constants
/// @{

#define ANKI_CFG_SCENE_NODES_AVERAGE_COUNT 1024

/// 1MB
#define ANKI_CFG_SCENE_ALLOCATOR_SIZE 0x100000 

// 512K
#define ANKI_CFG_SCENE_FRAME_ALLOCATOR_SIZE 0x80000

/// @{
/// Used to optimize the initial vectors of VisibilityTestResults
#define ANKI_CFG_FRUSTUMABLE_AVERAGE_VISIBLE_RENDERABLES_COUNT 16
#define ANKI_CFG_FRUSTUMABLE_AVERAGE_VISIBLE_LIGHTS_COUNT 8
/// @}

/// If true then we can place spatials in a thread-safe way
#define ANKI_CFG_OCTREE_THREAD_SAFE 1

/// @}
/// @}

/// @addtogroup Scene
/// @{

/// The type of the scene's allocator
template<typename T>
using SceneAllocator = StackAllocator<T, false>;

/// The type of the scene's frame allocator
template<typename T>
using SceneFrameAllocator = StackAllocator<T, false>;

/// Scene string
typedef std::basic_string<char, std::char_traits<char>,
	SceneAllocator<char>> SceneString;

/// Scene vector
template<typename T>
using SceneVector = Vector<T, SceneAllocator<T>>;

/// The same as SceneVector. Different name to show the difference
template<typename T>
using SceneFrameVector = Vector<T, SceneFrameAllocator<T>>;

/// Deleter for shared pointers
template<typename T>
struct SceneSharedPtrDeleter
{
	void operator()(T* x)
	{
		ANKI_ASSERT(x);
		SceneAllocator<U8> alloc = x->getSceneAllocator();
		//ANKI_DELETE(x, alloc);
	}
};

/// @}

} // end namespace anki

#endif
