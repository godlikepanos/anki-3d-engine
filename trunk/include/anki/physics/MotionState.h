// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_MOTION_STATE_H
#define ANKI_PHYSICS_MOTION_STATE_H

#include "anki/Math.h"
#include "anki/physics/Converters.h"
#include <LinearMath/btMotionState.h>

namespace anki {

/// A custom and generic bullet motion state
class MotionState: public btMotionState
{
public:
	MotionState()
		: MotionState(toBt(Transform::getIdentity()))
	{}

	MotionState(const btTransform& worldTransform_)
		: worldTransform(worldTransform_), updated(false)
	{}

	~MotionState()
	{}

	MotionState& operator=(const MotionState& b)
	{
		worldTransform = b.worldTransform;
		updated = b.updated;
		return *this;
	}

	/// @name Accessors
	/// @{
	Bool getUpdated() const
	{
		return updated;
	}
	void setUpdated(Bool x)
	{
		updated = x;
	}
	const btTransform& getWorldTransform2() const
	{
		return worldTransform;
	}
	/// @}

	/// @name Bullet implementation of virtuals
	/// @{
	void getWorldTransform(btTransform& out) const
	{
		out = worldTransform;
	}

	void setWorldTransform(const btTransform& in)
	{
		worldTransform = in;
		updated = true;
	}
	/// @}

private:
	btTransform worldTransform;
	Bool8 updated;
};

} // end namespace anki

#endif
