// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

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
SceneObject::~SceneObject()
{
	scene->decreaseObjectsMarkedForDeletion();
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
	// Mark for deletion only when it's not already marked because we don't 
	// want to increase the counter again
	if(!isMarkedForDeletion())
	{
		flags |= MARKED_FOR_DELETION;
		scene->increaseObjectsMarkedForDeletion();
	}

	visitChildren([](SceneObject& obj)
	{
		obj.markForDeletion();
	});
}

} // end namespace anki
