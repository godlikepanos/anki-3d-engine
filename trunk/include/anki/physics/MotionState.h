#ifndef ANKI_PHYSICS_MOTION_STATE_H
#define ANKI_PHYSICS_MOTION_STATE_H

#include <bullet/LinearMath/btMotionState.h>
#include "anki/scene/Movable.h"

namespace anki {

/// A custom motion state
class MotionState: public btMotionState
{
public:
	MotionState(const Transform& initialTransform, Movable* node_)
		: worldTransform(toBt(initialTransform)), node(node_)
	{}

	~MotionState() {}

	/// @name Bullet implementation of virtuals
	/// @{
	void getWorldTransform(btTransform& worldTrans) const
	{
		worldTrans = worldTransform;
	}

	const btTransform& getWorldTransform() const
	{
		return worldTransform;;
	}

	void setWorldTransform(const btTransform& worldTrans)
	{
		worldTransform = worldTrans;

		if(node)
		{
			Transform& nodeTrf = node->getLocalTransform();
			float originalScale = nodeTrf.getScale();
			nodeTrf = toAnki(worldTrans);
			nodeTrf.setScale(originalScale);
		}
	}
	/// @}

private:
	btTransform worldTransform;
	Movable* node; ///< Pointer cause it may be NULL
};

} // end namespace

#endif
