// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/MoveComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
MoveComponent::MoveComponent(SceneNode* node, U32 flags)
	:	SceneComponent(MOVE_COMPONENT, node),
		Base(nullptr, node->getSceneAllocator()),
		Bitset<U8>(flags),
		m_node(node)
{
	markForUpdate();
}

//==============================================================================
MoveComponent::~MoveComponent()
{}

//==============================================================================
Bool MoveComponent::update(SceneNode&, F32, F32, UpdateType uptype)
{
	if(uptype == ASYNC_UPDATE && getParent() == nullptr)
	{
		// Call this only on roots
		updateWorldTransform();
	}

	// move component does it's own updates
	return false;
}

//==============================================================================
void MoveComponent::updateWorldTransform()
{
	m_prevWTrf = m_wtrf;
	const Bool dirty = bitsEnabled(MF_MARKED_FOR_UPDATE);

	// If dirty then update world transform
	if(dirty)
	{
		const MoveComponent* parent = getParent();

		if(parent)
		{
			if(bitsEnabled(MF_IGNORE_LOCAL_TRANSFORM))
			{
				m_wtrf = parent->getWorldTransform();
			}
			else
			{
				m_wtrf = 
					parent->getWorldTransform().combineTransformations(m_ltrf);
			}
		}
		else
		{
			m_wtrf = m_ltrf;
		}

		m_node->componentUpdated(*this, ASYNC_UPDATE);
		timestamp = getGlobTimestamp();

		// Now it's a good time to cleanse parent
		disableBits(MF_MARKED_FOR_UPDATE);
	}

	// Update the children
	visitChildren([&](MoveComponent& mov)
	{
		// If parent is dirty then make children dirty as well
		if(dirty)
		{
			mov.markForUpdate();
		}

		mov.updateWorldTransform();
	});
}

} // end namespace anki
