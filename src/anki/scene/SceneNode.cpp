// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

SceneNode::SceneNode(SceneGraph* scene, CString name)
	: m_scene(scene)
	, m_uuid(scene->getNewUuid())
{
	if(name)
	{
		m_name.create(getSceneAllocator(), name);
	}
}

SceneNode::~SceneNode()
{
	auto alloc = getSceneAllocator();

	auto it = m_components.getBegin();
	auto end = m_components.getEnd();
	for(; it != end; ++it)
	{
		SceneComponent* comp = *it;
		alloc.deleteInstance(comp);
	}

	Base::destroy(alloc);
	m_name.destroy(alloc);
	m_components.destroy(alloc);
}

void SceneNode::setMarkedForDeletion()
{
	// Mark for deletion only when it's not already marked because we don't want to increase the counter again
	if(!getMarkedForDeletion())
	{
		m_flags.set(Flag::MARKED_FOR_DELETION);
		m_scene->increaseObjectsMarkedForDeletion();
	}

	Error err = visitChildren([](SceneNode& obj) -> Error {
		obj.setMarkedForDeletion();
		return Error::NONE;
	});

	(void)err;
}

Timestamp SceneNode::getGlobalTimestamp() const
{
	return m_scene->getGlobalTimestamp();
}

SceneAllocator<U8> SceneNode::getSceneAllocator() const
{
	ANKI_ASSERT(m_scene);
	return m_scene->getAllocator();
}

SceneFrameAllocator<U8> SceneNode::getFrameAllocator() const
{
	ANKI_ASSERT(m_scene);
	return m_scene->getFrameAllocator();
}

ResourceManager& SceneNode::getResourceManager()
{
	return m_scene->getResourceManager();
}

} // end namespace anki
