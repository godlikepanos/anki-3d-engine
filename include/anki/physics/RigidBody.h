#ifndef ANKI_PHYSICS_RIGID_BODY_H
#define ANKI_PHYSICS_RIGID_BODY_H

#include "anki/physics/PhysicsObject.h"
#include "anki/scene/SceneComponent.h"
#include "anki/Math.h"
#include "anki/physics/MotionState.h"
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

namespace anki {

class MoveComponent;

/// Wrapper for rigid body
class RigidBody: public PhysicsObject, public btRigidBody, 
	public SceneComponent
{
	friend class PhysicsWorld;

public:
	/// Initializer class
	struct Initializer
	{
		F32 mass = 0.0;
		Transform startTrf = Transform::getIdentity();
		btCollisionShape* shape = nullptr;
		I32 group = -1;
		I32 mask = -1;
		Bool kinematic = false;
	};

	/// Init and register. Only the PhysicsWorld can construct it
	RigidBody(PhysicsWorld* world, const Initializer& init);

	/// Unregister. Only the PhysicsWorld can destroy it
	~RigidBody();

	Bool syncUpdate(SceneNode& node, F32 prevTime, F32 crntTime);

private:
	MotionState motionState;
};

} // end namespace anki

#endif