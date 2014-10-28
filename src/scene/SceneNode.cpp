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
Error SceneNode::create(const CString& name, U32 maxComponents)
{
	Error err = ErrorCode::NONE;
	
	if(maxComponents)
	{
		err = m_components.create(getSceneAllocator(), maxComponents);
	}

	if(!err && !name.isEmpty())
	{
		err = m_name.create(getSceneAllocator(), name);
	}

	return err;
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
void SceneNode::addComponent(SceneComponent* comp)
{
	ANKI_ASSERT(comp);
	m_components[m_componentsCount++] = comp;
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
