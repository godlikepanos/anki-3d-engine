#include "RigidBody.h"
#include "App.h"
#include "Physics.h"
#include "Scene.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
RigidBody::RigidBody(float mass, const Transform& startTransform, btCollisionShape* shape, SceneNode* node,
                     int group, int mask, Object* parent):
  btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0.0, NULL, NULL, btVector3(0.0, 0.0, 0.0))),
	Object(parent)
{
	DEBUG_ERR(shape==NULL || shape->getShapeType()==INVALID_SHAPE_PROXYTYPE);

	bool isDynamic = (mass != 0.0);

	btVector3 localInertia(0.0, 0.0, 0.0);
	if(isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	MotionState* myMotionState = new MotionState(toBt(startTransform), *node, this);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

	setupRigidBody(cInfo);

	setContactProcessingThreshold(defaultContactProcessingThreshold);

	if(mask==-1 || group==-1)
		app->getScene()->getPhysics()->getDynamicsWorld()->addRigidBody(this);
	else
		app->getScene()->getPhysics()->getDynamicsWorld()->addRigidBody(this, group, mask);
}
