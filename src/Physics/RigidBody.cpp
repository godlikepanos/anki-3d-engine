#include "RigidBody.h"
#include "Physics.h"
#include "Scene.h"
#include "MotionState.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
RigidBody::RigidBody(Physics& physics_, const Initializer& init):
  btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0.0, NULL, NULL, btVector3(0.0, 0.0, 0.0))), // dummy init
  physics(physics_)
{
	DEBUG_ERR(init.shape==NULL || init.shape->getShapeType()==INVALID_SHAPE_PROXYTYPE);

	bool isDynamic = (init.mass != 0.0);

	btVector3 localInertia;
	if(isDynamic)
		init.shape->calculateLocalInertia(init.mass, localInertia);
	else
		localInertia = btVector3(0.0, 0.0, 0.0);

	motionState.reset(new MotionState(toBt(init.startTrf), init.sceneNode));

	btRigidBody::btRigidBodyConstructionInfo cInfo(init.mass, motionState.get(), init.shape, localInertia);

	setupRigidBody(cInfo);

	setContactProcessingThreshold(physics.defaultContactProcessingThreshold);

	forceActivationState(ISLAND_SLEEPING);

	if(init.mask==-1 || init.group==-1)
		physics.dynamicsWorld->addRigidBody(this);
	else
		physics.dynamicsWorld->addRigidBody(this, init.group, init.mask);
}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
RigidBody::~RigidBody()
{
	physics.dynamicsWorld->removeRigidBody(this);
}
