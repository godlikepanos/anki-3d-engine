// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SceneNode.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
SceneNode::SceneNode(SceneGraph* scene)
:	SceneObject(Type::SCENE_NODE, scene)
{}

//==============================================================================
Error SceneNode::create(const CString& name)
{
	return m_name.create(getSceneAllocator(), name);
}

//==============================================================================
SceneNode::~SceneNode()
{
	auto alloc = getSceneAllocator();
	m_name.destroy(alloc);
	m_components.destroy(alloc);
}

//==============================================================================
U32 SceneNode::getLastUpdateFrame() const
{
	U32 max = 0;

	Error err = iterateComponents([&max](const SceneComponent& comp) -> Error
	{
		max = std::max(max, comp.getTimestamp());
		return ErrorCode::NONE;
	});

	(void)err;

	return max;
}

//==============================================================================
Error SceneNode::addComponent(SceneComponent* comp)
{
	ANKI_ASSERT(comp);
	Error err = ErrorCode::NONE;

	if(m_components.getSize() < m_componentsCount + 1)
	{
		// Not enough room
		const U extra = 2;
		err = m_components.resize(
			getSceneAllocator(), m_componentsCount + 1 + extra);
	}

	if(!err)
	{
		m_components[m_componentsCount++] = comp;
	}

	return err;
}

//==============================================================================
void SceneNode::removeComponent(SceneComponent* comp)
{
	ANKI_ASSERT(0 && "TODO");
}

//==============================================================================
ResourceManager& SceneNode::getResourceManager()
{
	return getSceneGraph()._getResourceManager();
}

} // end namespace anki
