#include "anki/physics/MotionState.h"
#include "anki/physics/Converters.h"

namespace anki {

//==============================================================================
MotionState::MotionState()
{
	worldTransform = toBt(Transform::getIdentity());
	node = nullptr;
	needsUpdate = true;
}

//==============================================================================
MotionState::MotionState(const Transform& initialTransform, 
	MoveComponent* node_)
	: worldTransform(toBt(initialTransform)), node(node_)
{
	needsUpdate = true;
}

//==============================================================================
MotionState& MotionState::operator=(const MotionState& b)
{
	worldTransform = b.worldTransform;
	node = b.node;
	return *this;
}

//==============================================================================
void MotionState::getWorldTransform(btTransform& worldTrans) const
{
	worldTrans = worldTransform;
}

//==============================================================================
void MotionState::setWorldTransform(const btTransform& worldTrans)
{
	worldTransform = worldTrans;
	needsUpdate = true;
}

//==============================================================================
void MotionState::sync()
{
	if(node && needsUpdate)
	{
		// Set local transform and preserve scale
		Transform newTrf;
		F32 originalScale = node->getLocalTransform().getScale();
		newTrf = toAnki(worldTransform);
		newTrf.setScale(originalScale);
		node->setLocalTransform(newTrf);

		// Set the flag
		needsUpdate = false;
	}
}

} // end namespace anki
