#include "anki/scene/MoveComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
MoveComponent::MoveComponent(SceneNode* node, U32 flags)
	:	Base(nullptr, node->getSceneAllocator()),
		Bitset<U8>(flags)
{
	markForUpdate();
}

//==============================================================================
MoveComponent::~MoveComponent()
{}

//==============================================================================
void MoveComponent::update()
{
	// Call this only on roots
	if(getParent() == nullptr)
	{
		// If root begin updating
		updateWorldTransform();
	}
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

		moveUpdate();
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

	// Now it's a good time to cleanse parent
	disableBits(MF_MARKED_FOR_UPDATE);
	timestamp = getGlobTimestamp();
}

} // end namespace anki
