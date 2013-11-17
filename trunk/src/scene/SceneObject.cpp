#include "anki/scene/SceneObject.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
SceneObject::SceneObject(SceneObject* parent, SceneGraph* scene_)
	:	Base(parent, scene_->getAllocator()),
		scene(scene_),
		markedForDeletion(false)
{
	ANKI_ASSERT(scene);
}

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

//==============================================================================
void SceneObject::markForDeletion()
{
	markedForDeletion = true;

	visitChildren([](SceneObject& obj)
	{
		obj.markForDeletion();
	});
}

} // end namespace anki
