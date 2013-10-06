#ifndef ANKI_PHYSICS_MOTION_STATE_H
#define ANKI_PHYSICS_MOTION_STATE_H

#include "anki/scene/MoveComponent.h"
#include <LinearMath/btMotionState.h>

namespace anki {

/// A custom motion state
class MotionState: public btMotionState
{
public:
	MotionState();
	MotionState(const Transform& initialTransform, MoveComponent* node);

	~MotionState()
	{}

	MotionState& operator=(const MotionState& b);

	/// @name Bullet implementation of virtuals
	/// @{
	void getWorldTransform(btTransform& worldTrans) const
	{
		worldTrans = worldTransform;
	}

	void setWorldTransform(const btTransform& worldTrans);
	/// @}

private:
	btTransform worldTransform;
	MoveComponent* node; ///< Pointer cause it may be NULL
};

} // end namespace anki

#endif
