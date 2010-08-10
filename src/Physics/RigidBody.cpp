#include "RigidBody.h"
#include "App.h"
#include "Physics.h"
#include "Scene.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
RigidBody::RigidBody(float mass, const Transform& startTransform, btCollisionShape* shape, SceneNode* node,
                     int group, int mask):
  btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0.0, NULL, NULL, btVector3(0.0, 0.0, 0.0))) // dummy init
{
	DEBUG_ERR(shape==NULL || shape->getShapeType()==INVALID_SHAPE_PROXYTYPE);

	bool isDynamic = (mass != 0.0);

	btVector3 localInertia(0.0, 0.0, 0.0);
	if(isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	motionState.reset(new MotionState(toBt(startTransform), *node));

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState.get(), shape, localInertia);

	setupRigidBody(cInfo);

	setContactProcessingThreshold(app->getScene()->getPhysics()->getDefaultContactProcessingThreshold());

	if(mask==-1 || group==-1)
		app->getScene()->getPhysics()->getDynamicsWorld()->addRigidBody(this);
	else
		app->getScene()->getPhysics()->getDynamicsWorld()->addRigidBody(this, group, mask);
}


//======================================================================================================================
// RigidBody                                                                                                           =
//======================================================================================================================
RigidBody::~RigidBody()
{
	// find the body
	RigidBody* body = NULL;
	btCollisionObject* obj;
	int i;
	for(i=0; i<app->getScene()->getPhysics()->getDynamicsWorld()->getNumCollisionObjects(); i++)
	{
		obj = app->getScene()->getPhysics()->getDynamicsWorld()->getCollisionObjectArray()[i];
		RigidBody* body1 = static_cast<RigidBody*>(btRigidBody::upcast(obj)); // from btCollisionShapt to RigidBody

		if(body1 == this)
		{
			body = body1;
			break;
		}
	}

	// check
	if(body == NULL)
	{
		ERROR("Cannot find body 0x" << hex << body);
		return;
	}

	// remove it from world
	app->getScene()->getPhysics()->getDynamicsWorld()->removeCollisionObject(obj);
}
