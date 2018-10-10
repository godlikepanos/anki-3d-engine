// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/SceneComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

SceneComponent::SceneComponent(SceneComponentType type, SceneNode* node)
	: m_type(type)
	, m_node(node)
	, m_uuid(node->getSceneGraph().getNewUuid())
	, m_idx(node->getComponentCount())
{
}

SceneComponent::~SceneComponent()
{
}

Timestamp SceneComponent::getGlobalTimestamp() const
{
	return m_node->getGlobalTimestamp();
}

Error SceneComponent::updateReal(Second prevTime, Second crntTime, Bool& updated)
{
	Error err = update(prevTime, crntTime, updated);
	if(!err && updated)
	{
		m_timestamp = getGlobalTimestamp();
	}

	return err;
}

SceneGraph& SceneComponent::getSceneGraph()
{
	return m_node->getSceneGraph();
}

const SceneGraph& SceneComponent::getSceneGraph() const
{
	return m_node->getSceneGraph();
}

SceneAllocator<U8> SceneComponent::getAllocator() const
{
	return m_node->getSceneAllocator();
}

SceneFrameAllocator<U8> SceneComponent::getFrameAllocator() const
{
	return m_node->getFrameAllocator();
}

} // end namespace anki
