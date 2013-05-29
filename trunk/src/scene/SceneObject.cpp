#include "anki/scene/SceneObject.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

#if 0

//==============================================================================
SceneObject::SceneObject(SceneObject* parent, SceneGraph* scene_)
	: Base(parent, scene_->getAllocator()), scene(scene_)
{}

//==============================================================================
SceneAllocator<U8> SceneObject::getSceneAllocator() const
{
	ANKI_ASSERT(scene);
	return scene->getAllocator();
}

//==============================================================================
SceneAllocator<U8> SceneObject::getSceneFrameAllocator() const
{
	ANKI_ASSERT(scene);
	return scene->getFrameAllocator();
}

#endif

} // end namespace anki
