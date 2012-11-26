#include "anki/physics/MotionState.h"
#include "anki/physics/Converters.h"

namespace anki {

//==============================================================================
MotionState::MotionState()
{
	worldTransform = toBt(Transform::getIdentity());
	node = nullptr;
}

//==============================================================================
MotionState::MotionState(const Transform& initialTransform, Movable* node_)
	: worldTransform(toBt(initialTransform)), node(node_)
{}

//==============================================================================
MotionState& MotionState::operator=(const MotionState& b)
{
	worldTransform = b.worldTransform;
	node = b.node;
	return *this;
}

//==============================================================================
void MotionState::setWorldTransform(const btTransform& worldTrans)
{
	worldTransform = worldTrans;

	if(node)
	{
		// Set local transform and preserve scale
		Transform newTrf;
		F32 originalScale = node->getLocalTransform().getScale();
		newTrf = toAnki(worldTrans);
		newTrf.setScale(originalScale);
		node->setLocalTransform(newTrf);
	}
}

} // end namespace anki
