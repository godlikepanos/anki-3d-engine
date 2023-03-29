// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>

namespace anki {

SceneNode::SceneNode(CString name)
	: m_uuid(SceneGraph::getSingleton().getNewUuid())
{
	if(name)
	{
		m_name = name;
	}

	// Add the implicit MoveComponent
	newComponent<MoveComponent>();
}

SceneNode::~SceneNode()
{
	for(SceneComponent* comp : m_components)
	{
		comp->onDestroyReal(*this);
		g_sceneComponentCallbacks.m_destructor[comp->getClassId()](*comp);

		SceneMemoryPool::getSingleton().free(comp);
	}

	Base::destroy(SceneMemoryPool::getSingleton());
}

void SceneNode::setMarkedForDeletion()
{
	// Mark for deletion only when it's not already marked because we don't want to increase the counter again
	if(!getMarkedForDeletion())
	{
		m_markedForDeletion = true;
		SceneGraph::getSingleton().increaseObjectsMarkedForDeletion();
	}

	[[maybe_unused]] const Error err = visitChildren([](SceneNode& obj) -> Error {
		obj.setMarkedForDeletion();
		return Error::kNone;
	});
}

void SceneNode::newComponentInternal(SceneComponent* newc)
{
	m_componentTypeMask |= 1 << newc->getClassId();

	// Inform all other components that some component was added
	for(SceneComponent* other : m_components)
	{
		other->onOtherComponentRemovedOrAddedReal(newc, true);
	}

	// Inform the current component about others
	for(SceneComponent* other : m_components)
	{
		newc->onOtherComponentRemovedOrAddedReal(other, true);
	}

	m_components.emplaceBack(newc);

	// Sort based on update weight
	std::sort(m_components.getBegin(), m_components.getEnd(), [](const SceneComponent* a, const SceneComponent* b) {
		const F32 weightA = a->getClassRtti().m_updateWeight;
		const F32 weightB = b->getClassRtti().m_updateWeight;
		if(weightA != weightB)
		{
			return weightA < weightB;
		}
		else
		{
			return a->getClassId() < b->getClassId();
		}
	});
}

Bool SceneNode::updateTransform()
{
	const Bool needsUpdate = m_localTransformDirty;
	m_localTransformDirty = false;
	const Bool updatedLastFrame = m_transformUpdatedThisFrame;
	m_transformUpdatedThisFrame = needsUpdate;

	if(needsUpdate || updatedLastFrame)
	{
		m_prevWTrf = m_wtrf;
	}

	// Update world transform
	if(needsUpdate)
	{
		const SceneNode* parent = getParent();

		if(parent == nullptr || m_ignoreParentNodeTransform)
		{
			m_wtrf = m_ltrf;
		}
		else
		{
			m_wtrf = parent->getWorldTransform().combineTransformations(m_ltrf);
		}

		// Make children dirty as well. Don't walk the whole tree because you will re-walk it later
		[[maybe_unused]] const Error err = visitChildrenMaxDepth(1, [](SceneNode& childNode) -> Error {
			childNode.m_localTransformDirty = true;
			return Error::kNone;
		});
	}

	return needsUpdate;
}

} // end namespace anki
