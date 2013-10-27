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
	void getWorldTransform(btTransform& worldTrans) const;

	void setWorldTransform(const btTransform& worldTrans);
	/// @}

	/// This function will actualy update the transform of the node. It is 
	/// called
	void sync();

private:
	btTransform worldTransform;
	MoveComponent* node; ///< Pointer cause it may be NULL
	Bool8 needsUpdate;
};

} // end namespace anki

#endif
