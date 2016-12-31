// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/SceneComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

SceneComponent::SceneComponent(SceneComponentType type, SceneNode* node)
	: m_node(node)
	, m_type(type)
{
	m_node->getSceneGraph().getSceneComponentLists().insertNew(this);
}

SceneComponent::~SceneComponent()
{
	m_node->getSceneGraph().getSceneComponentLists().remove(this);
}

Timestamp SceneComponent::getGlobalTimestamp() const
{
	return m_node->getGlobalTimestamp();
}

Error SceneComponent::updateReal(SceneNode& node, F32 prevTime, F32 crntTime, Bool& updated)
{
	Error err = update(node, prevTime, crntTime, updated);
	if(!err && updated)
	{
		err = onUpdate(node, prevTime, crntTime);

		if(!err)
		{
			m_timestamp = getGlobalTimestamp();
		}
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

void SceneComponentLists::insertNew(SceneComponent* comp)
{
	ANKI_ASSERT(comp);
	ANKI_ASSERT(find(comp) == m_lists[comp->getType()].getEnd());

	m_lists[comp->getType()].pushBack(m_alloc, comp);
}

void SceneComponentLists::remove(SceneComponent* comp)
{
	ANKI_ASSERT(comp);

	auto it = find(comp);
	ANKI_ASSERT(it != m_lists[comp->getType()].getEnd());
	m_lists[comp->getType()].erase(m_alloc, it);
}

List<SceneComponent*>::Iterator SceneComponentLists::find(SceneComponent* comp)
{
	ANKI_ASSERT(comp);

	List<SceneComponent*>& list = m_lists[comp->getType()];
	auto it = list.getBegin();
	auto end = list.getEnd();
	while(it != end)
	{
		if(*it == comp)
		{
			break;
		}
		++it;
	}

	return it;
}

} // end namespace anki
