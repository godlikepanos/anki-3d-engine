#ifndef ANKI_PHYSICS_RIGID_BODY_H
#define ANKI_PHYSICS_RIGID_BODY_H

#include "anki/Math.h"
#include "anki/physics/MotionState.h"
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

namespace anki {

class Movable;
class PhysWorld;

/// Wrapper for rigid body
class RigidBody: public btRigidBody
{
public:
	/// Initializer class
	struct Initializer
	{
		F32 mass = 0.0;
		Transform startTrf = Transform::getIdentity();
		btCollisionShape* shape = nullptr;
		Movable* movable = nullptr;
		I32 group = -1;
		I32 mask = -1;
	};

	/// Init and register
	RigidBody(PhysWorld* masterContainer, const Initializer& init, 
		Movable* movable = nullptr);

	/// Unregister
	~RigidBody();

private:
	PhysWorld* masterContainer; ///< Know your father
	MotionState motionState;
};

} // end namespace anki

#endif
