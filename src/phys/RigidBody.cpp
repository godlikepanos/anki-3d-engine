#include "RigidBody.h"
#include "MasterContainer.h"
#include "scene/Scene.h"
#include "MotionState.h"


namespace phys {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
RigidBody::Initializer::Initializer():
	mass(0.0),
	startTrf(Transform::getIdentity()),
	shape(NULL),
	sceneNode(NULL),
	group(-1),
	mask(-1)
{}


//==============================================================================
// Constructor                                                                 =
//==============================================================================
RigidBody::RigidBody(MasterContainer& masterContainer_,
	const Initializer& init)
:	btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0.0, NULL, NULL,
		btVector3(0.0, 0.0, 0.0))), // dummy init
	masterContainer(masterContainer_)
{
	ASSERT(init.shape != NULL &&
		init.shape->getShapeType() != INVALID_SHAPE_PROXYTYPE);

	bool isDynamic = (init.mass != 0.0);

	btVector3 localInertia;
	if(isDynamic)
	{
		init.shape->calculateLocalInertia(init.mass, localInertia);
	}
	else
	{
		localInertia = btVector3(0.0, 0.0, 0.0);
	}

	motionState.reset(new MotionState(init.startTrf, init.sceneNode));

	btRigidBody::btRigidBodyConstructionInfo cInfo(init.mass,
		motionState.get(), init.shape, localInertia);

	setupRigidBody(cInfo);

	setContactProcessingThreshold(
		masterContainer.defaultContactProcessingThreshold);

	forceActivationState(ISLAND_SLEEPING);

	// register
	if(init.mask == -1 || init.group == -1)
	{
		masterContainer.dynamicsWorld->addRigidBody(this);
	}
	else
	{
		masterContainer.dynamicsWorld->addRigidBody(this, init.group,
			init.mask);
	}
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
RigidBody::~RigidBody()
{
	masterContainer.dynamicsWorld->removeRigidBody(this);
}


} // end namespace
