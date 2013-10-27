#include "anki/physics/RigidBody.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/scene/SceneGraph.h"
#include "anki/physics/MotionState.h"

namespace anki {

//==============================================================================
RigidBody::RigidBody(PhysicsWorld* world, const Initializer& init)
	:	PhysicsObject(world),
		btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0.0, nullptr, 
			nullptr, btVector3(0.0, 0.0, 0.0))) // dummy init
{
	ANKI_ASSERT(init.shape != nullptr 
		&& init.shape->getShapeType() != INVALID_SHAPE_PROXYTYPE);

	Bool isDynamic = (init.mass != 0.0);

	btVector3 localInertia;
	if(isDynamic)
	{
		init.shape->calculateLocalInertia(init.mass, localInertia);
	}
	else
	{
		localInertia = btVector3(0.0, 0.0, 0.0);
	}

	motionState = MotionState(init.startTrf, init.movable);

	btRigidBody::btRigidBodyConstructionInfo cInfo(
		init.mass, &motionState, init.shape, localInertia);

	setupRigidBody(cInfo);

	setContactProcessingThreshold(
		getPhysicsWorld().defaultContactProcessingThreshold);

	//forceActivationState(ISLAND_SLEEPING);

	// register
	if(init.mask == -1 || init.group == -1)
	{
		getPhysicsWorld().dynamicsWorld->addRigidBody(this);
	}
	else
	{
		getPhysicsWorld().dynamicsWorld->addRigidBody(
			this, init.group, init.mask);
	}
}

//==============================================================================
RigidBody::~RigidBody()
{
	getPhysicsWorld().dynamicsWorld->removeRigidBody(this);
}

} // end namespace anki
