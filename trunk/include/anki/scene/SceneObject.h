#ifndef ANKI_SCENE_SCENE_OBJECT_H
#define ANKI_SCENE_SCENE_OBJECT_H

#include "anki/scene/Common.h"
#include "anki/util/Object.h"

namespace anki {

// Forward
class SceneGraph;

/// The SceneObject deleter
template<typename T, typename Alloc>
struct SceneObjectDeleter
{
	void operator()(T* x)
	{
		Alloc alloc = x->getSceneAllocator();

		alloc.destroy(x);
		alloc.deallocate(x, 1);
	}
};

/// The base of all scene related objects
class SceneObject: public Object<SceneObject, SceneAllocator<SceneObject>,
	SceneObjectDeleter<SceneObject, SceneAllocator<SceneObject>>>
{
public:
	typedef Object<SceneObject, SceneAllocator<SceneObject>,
		SceneObjectDeleter<SceneObject, SceneAllocator<SceneObject>>> Base;

	SceneObject(SceneObject* parent, SceneGraph* scene);

	SceneAllocator<U8> getSceneAllocator() const;

	SceneAllocator<U8> getSceneFrameAllocator() const;

	SceneGraph& getSceneGraph()
	{
		return *scene;
	}

public:
	SceneGraph* scene;
};

} // end namespace anki

#endif
