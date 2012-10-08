#ifndef ANKI_PHYSICS_RIGID_BODY_H
#define ANKI_PHYSICS_RIGID_BODY_H

#include "anki/math/Math.h"
#include <memory>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

namespace anki {

class Movable;
class MotionState;
class PhysWorld;

/// Wrapper for rigid body
class RigidBody: public btRigidBody
{
public:
	/// Initializer class
	struct Initializer
	{
		float mass;
		Transform startTrf;
		btCollisionShape* shape;
		Movable* movable;
		int group;
		int mask;

		Initializer()
			: mass(0.0), startTrf(Transform::getIdentity()),
				shape(nullptr), movable(nullptr), group(-1), mask(-1)
		{}
	};

	/// Init and register
	RigidBody(PhysWorld* masterContainer, const Initializer& init);

	/// Unregister
	~RigidBody();

private:
	PhysWorld* masterContainer; ///< Know your father
	/// Keep it here for garbage collection
	std::unique_ptr<MotionState> motionState;
};

} // end namespace

#endif
