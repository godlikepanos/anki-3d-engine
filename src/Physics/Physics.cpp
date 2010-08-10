#include "Physics.h"



//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Physics::Physics():
	defaultContactProcessingThreshold(BT_LARGE_FLOAT)
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new	btCollisionDispatcher(collisionConfiguration);
	broadphase = new btDbvtBroadphase();
	sol = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, sol, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,-10, 0));

	debugDrawer = new DebugDrawer;
	dynamicsWorld->setDebugDrawer(debugDrawer);
	dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
}


//======================================================================================================================
// createNewRigidBody                                                                                                  =
//======================================================================================================================
btRigidBody* Physics::createNewRigidBody(float mass, const Transform& startTransform, btCollisionShape* shape,
                                         SceneNode* node, int group, int mask)
{
	DEBUG_ERR(shape==NULL || shape->getShapeType()==INVALID_SHAPE_PROXYTYPE);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.0);

	btVector3 localInertia(0, 0, 0);
	if(isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, NULL, shape, localInertia);

	btRigidBody* body = new btRigidBody(cInfo);

	MotionState* myMotionState = new MotionState(toBt(startTransform), *node/*, body*/);

	body->setMotionState(myMotionState);

	body->setContactProcessingThreshold(defaultContactProcessingThreshold);

	if(mask==-1 || group==-1)
		dynamicsWorld->addRigidBody(body);
	else
		dynamicsWorld->addRigidBody(body, group, mask);

	return body;
}


//======================================================================================================================
// deleteRigidBody                                                                                                     =
//======================================================================================================================
void Physics::deleteRigidBody(btRigidBody* body)
{
	/*// find the body
	btRigidBody* body1 = NULL;
	btCollisionObject* obj;
	int i;
	for(i=0; i<dynamicsWorld->getNumCollisionObjects(); i++)
	{
		obj = dynamicsWorld->getCollisionObjectArray()[i];
		body1 = btRigidBody::upcast(obj);

		if(body1 == body)
			break;
	}

	// check
	if(body1 == NULL)
	{
		ERROR("Cannot find body 0x" << hex << body);
		return;
	}

	// remove it from world
	dynamicsWorld->removeCollisionObject(obj);

	// delete motion state
	if (body && body->getMotionState())
	{
		delete body->getMotionState();
	}*/
}
