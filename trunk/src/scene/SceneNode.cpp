// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SceneNode.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
SceneNode::SceneNode(const CString& name, SceneGraph* scene)
:	SceneObject(SCENE_NODE_TYPE, nullptr, scene),
	m_name(getSceneAllocator()),
	m_components(getSceneAllocator())
{
	ANKI_ASSERT(scene);

	m_components.reserve(2);

	if(!name.isEmpty())
	{
		m_name = name;
	}
}

//==============================================================================
SceneNode::~SceneNode()
{
	SceneAllocator<SceneComponent*> alloc = getSceneAllocator();
	for(auto comp : m_components)
	{
		alloc.deleteInstance(comp);
	}
}

//==============================================================================
U32 SceneNode::getLastUpdateFrame() const
{
	U32 max = 0;

	iterateComponents([&](const SceneComponent& comp)
	{
		max = std::max(max, comp.getTimestamp());
	});

	return max;
}

//==============================================================================
void SceneNode::addComponent(SceneComponent* comp)
{
	ANKI_ASSERT(comp);
	m_components.push_back(comp);
}

//==============================================================================
void SceneNode::removeComponent(SceneComponent* comp)
{
	auto it = std::find(m_components.begin(), m_components.end(), comp);
	ANKI_ASSERT(it != m_components.end());
	m_components.erase(it);
}

//==============================================================================
ResourceManager& SceneNode::getResourceManager()
{
	return getSceneGraph()._getResourceManager();
}

} // end namespace anki
