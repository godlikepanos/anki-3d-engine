// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SceneObject.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
SceneObject::SceneObject(Type type, SceneObject* parent, SceneGraph* scene)
:	Base(),
	m_scene(scene),
	m_bits(static_cast<U8>(type))
{
	ANKI_ASSERT(m_scene);

	// TODO Remove that
	if(parent)
	{
		parent->addChild(this);
	}
}

//==============================================================================
SceneObject::~SceneObject()
{
	m_scene->decreaseObjectsMarkedForDeletion();
}

//==============================================================================
SceneAllocator<U8> SceneObject::getSceneAllocator() const
{
	ANKI_ASSERT(m_scene);
	return m_scene->getAllocator();
}

//==============================================================================
SceneAllocator<U8> SceneObject::getSceneFrameAllocator() const
{
	ANKI_ASSERT(m_scene);
	return m_scene->getFrameAllocator();
}

//==============================================================================
void SceneObject::markForDeletion()
{
	// Mark for deletion only when it's not already marked because we don't 
	// want to increase the counter again
	if(!isMarkedForDeletion())
	{
		m_bits |= enumToType(Flag::MARKED_FOR_DELETION);
		m_scene->increaseObjectsMarkedForDeletion();
	}

	visitChildren([](SceneObject& obj) -> Error
	{
		obj.markForDeletion();
		return ErrorCode::NONE;
	});
}

} // end namespace anki
