// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/MoveComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
MoveComponent::MoveComponent(SceneNode* node_, U32 flags)
	:	SceneComponent(MOVE_COMPONENT, node_),
		Base(nullptr, node_->getSceneAllocator()),
		Bitset<U8>(flags),
		node(node_)
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
	prevWTrf = wTrf;
	const Bool dirty = bitsEnabled(MF_MARKED_FOR_UPDATE);

	// If dirty then update world transform
	if(dirty)
	{
		const MoveComponent* parent = getParent();

		if(parent)
		{
			if(bitsEnabled(MF_IGNORE_LOCAL_TRANSFORM))
			{
				wTrf = parent->getWorldTransform();
			}
			else
			{
				wTrf = Transform::combineTransformations(
					parent->getWorldTransform(), lTrf);
			}
		}
		else
		{
			wTrf = lTrf;
		}

		node->componentUpdated(*this, ASYNC_UPDATE);
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
