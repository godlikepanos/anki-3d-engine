#ifndef ANKI_SCENE_COMMON_H
#define ANKI_SCENE_COMMON_H

#include "anki/util/Allocator.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Dictionary.h"
#include "anki/util/Object.h"
#include <memory>

namespace anki {

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

/// Scene dictionary
template<typename T>
using SceneDictionary = 
	Dictionary<T, SceneAllocator<std::pair<const char*, T>>>;

/// Deleter for shared pointers
template<typename T>
struct SceneSharedPtrDeleter
{
	void operator()(T* x)
	{
		ANKI_ASSERT(x);
		SceneAllocator<U8> alloc = x->getSceneAllocator();
		deleteObject(alloc, x);
	}
};

/// Shared pointer in scene
template<typename T>
using SceneSharedPointer = std::shared_ptr<T>;

/// Scene object
template<typename T>
using SceneHierarchicalObject = Object<T, SceneAllocator<T>>;

/// @}

} // end namespace anki

#endif
