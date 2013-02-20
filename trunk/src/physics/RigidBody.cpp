#include "anki/physics/RigidBody.h"
#include "anki/physics/PhysWorld.h"
#include "anki/scene/SceneGraph.h"
#include "anki/physics/MotionState.h"

namespace anki {

//==============================================================================
RigidBody::RigidBody(PhysWorld* masterContainer_, const Initializer& init,
	Movable* movable)
	: btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0.0, nullptr, 
		nullptr, btVector3(0.0, 0.0, 0.0))), // dummy init
		masterContainer(masterContainer_)
{
	ANKI_ASSERT(init.shape != nullptr 
		&& init.shape->getShapeType() != INVALID_SHAPE_PROXYTYPE);

	movable = (movable != nullptr) ? movable : init.movable;

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

	motionState = MotionState(init.startTrf, movable);

	btRigidBody::btRigidBodyConstructionInfo cInfo(
		init.mass, &motionState, init.shape, localInertia);

	setupRigidBody(cInfo);

	setContactProcessingThreshold(
		masterContainer->defaultContactProcessingThreshold);

	//forceActivationState(ISLAND_SLEEPING);

	// register
	if(init.mask == -1 || init.group == -1)
	{
		masterContainer->dynamicsWorld->addRigidBody(this);
	}
	else
	{
		masterContainer->dynamicsWorld->addRigidBody(
			this, init.group, init.mask);
	}
}

//==============================================================================
RigidBody::~RigidBody()
{
	masterContainer->dynamicsWorld->removeRigidBody(this);
}

} // end namespace anki
