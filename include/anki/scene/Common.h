#ifndef ANKI_SCENE_COMMON_H
#define ANKI_SCENE_COMMON_H

#include "anki/util/Allocator.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Scene configuration constants. The enum contains absolute values and other
/// constants that help to optimize allocations
struct SceneConfig
{
	enum 
	{
		SCENE_NODES_AVERAGE_COUNT = 1024,
		SCENE_ALLOCATOR_SIZE = 0x100000, // 1MB
		SCENE_FRAME_ALLOCATOR_SIZE = 0x80000, // 512K

		/// Used to optimize the initial vectors of VisibilityTestResults
		FRUSTUMABLE_AVERAGE_VISIBLE_SCENE_NODES_COUNT = 10
	};
};

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

/// @}

} // end namespace anki

#endif
