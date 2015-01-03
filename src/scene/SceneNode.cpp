// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SceneNode.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
SceneNode::SceneNode(SceneGraph* scene)
:	m_scene(scene)
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
	
	auto it = m_components.begin();
	auto end = m_components.begin() + m_componentsCount;
	for(; it != end; ++it)
	{
		SceneComponent* comp = *it;
		if(comp->getAutomaticCleanup())
		{
			alloc.deleteInstance(comp);
		}
	}

	Base::destroy(alloc);
	m_name.destroy(alloc);
	m_components.destroy(alloc);
}

//==============================================================================
void SceneNode::setMarkedForDeletion()
{
	// Mark for deletion only when it's not already marked because we don't 
	// want to increase the counter again
	if(!getMarkedForDeletion())
	{
		m_forDeletion = true;
		m_scene->increaseObjectsMarkedForDeletion();
	}

	Error err = visitChildren([](SceneNode& obj) -> Error
	{
		obj.setMarkedForDeletion();
		return ErrorCode::NONE;
	});

	(void)err;
}

//==============================================================================
SceneAllocator<U8> SceneNode::getSceneAllocator() const
{
	ANKI_ASSERT(m_scene);
	return m_scene->getAllocator();
}

//==============================================================================
SceneFrameAllocator<U8> SceneNode::getSceneFrameAllocator() const
{
	ANKI_ASSERT(m_scene);
	return m_scene->getFrameAllocator();
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
Error SceneNode::addComponent(SceneComponent* comp, Bool transferOwnership)
{
	ANKI_ASSERT(comp);
	Error err = ErrorCode::NONE;

#if ANKI_ASSERTIONS
	err = iterateComponents([&](const SceneComponent& bcomp) -> Error
	{
		ANKI_ASSERT(comp != &bcomp);
		return ErrorCode::NONE;
	});
#endif

	if(m_components.getSize() < m_componentsCount + 1u)
	{
		// Not enough room
		const U extra = 2;
		err = m_components.resize(
			getSceneAllocator(), m_componentsCount + 1 + extra);
	}

	if(!err)
	{
		m_components[m_componentsCount++] = comp;

		comp->setAutomaticCleanup(transferOwnership);
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
	return m_scene->_getResourceManager();
}

} // end namespace anki
