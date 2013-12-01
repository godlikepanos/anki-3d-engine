#include "anki/scene/SceneObject.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
SceneObject::SceneObject(Type type, SceneObject* parent, SceneGraph* scene_)
	:	Base(parent, scene_->getAllocator()),
		scene(scene_),
		flags((U8)type)
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
	flags |= MARKED_FOR_DELETION;

	visitChildren([](SceneObject& obj)
	{
		obj.markForDeletion();
	});
}

} // end namespace anki
