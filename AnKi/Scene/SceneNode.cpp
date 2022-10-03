// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

SceneNode::SceneNode(SceneGraph* scene, CString name)
	: m_scene(scene)
	, m_uuid(scene->getNewUuid())
{
	if(name)
	{
		m_name.create(getMemoryPool(), name);
	}
}

SceneNode::~SceneNode()
{
	HeapMemoryPool& pool = getMemoryPool();

	auto it = m_components.getBegin();
	auto end = m_components.getEnd();
	for(; it != end; ++it)
	{
		deleteInstance(pool, *it);
	}

	Base::destroy(pool);
	m_name.destroy(pool);
	m_components.destroy(pool);
	m_componentInfos.destroy(pool);
}

void SceneNode::setMarkedForDeletion()
{
	// Mark for deletion only when it's not already marked because we don't want to increase the counter again
	if(!getMarkedForDeletion())
	{
		m_markedForDeletion = true;
		m_scene->increaseObjectsMarkedForDeletion();
	}

	[[maybe_unused]] const Error err = visitChildren([](SceneNode& obj) -> Error {
		obj.setMarkedForDeletion();
		return Error::kNone;
	});
}

Timestamp SceneNode::getGlobalTimestamp() const
{
	return m_scene->getGlobalTimestamp();
}

HeapMemoryPool& SceneNode::getMemoryPool() const
{
	ANKI_ASSERT(m_scene);
	return m_scene->getMemoryPool();
}

StackMemoryPool& SceneNode::getFrameMemoryPool() const
{
	ANKI_ASSERT(m_scene);
	return m_scene->getFrameMemoryPool();
}

ResourceManager& SceneNode::getResourceManager()
{
	return m_scene->getResourceManager();
}

const ConfigSet& SceneNode::getConfig() const
{
	return m_scene->getConfig();
}

} // end namespace anki
