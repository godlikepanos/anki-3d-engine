#ifndef ANKI_SCENE_SCENE_OBJECT_H
#define ANKI_SCENE_SCENE_OBJECT_H

#include "anki/scene/Common.h"
#include "anki/util/Object.h"

namespace anki {

// Forward
class SceneGraph;

/// @addtogroup Scene
/// @{

/// The base of all scene related objects
class SceneObject: public Object<SceneObject, SceneAllocator<SceneObject>>
{
public:
	typedef Object<SceneObject, SceneAllocator<SceneObject>> Base;

	SceneObject(SceneObject* parent, SceneGraph* scene);

	SceneAllocator<U8> getSceneAllocator() const;

	SceneAllocator<U8> getSceneFrameAllocator() const;

	SceneGraph& getSceneGraph()
	{
		return *scene;
	}

	Bool isMarkedForDeletion() const
	{
		return markedForDeletion;
	}
	void markForDeletion();

public:
	SceneGraph* scene;
	Bool8 markedForDeletion;
};
/// @}

} // end namespace anki

#endif
