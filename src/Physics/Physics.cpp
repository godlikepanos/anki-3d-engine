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
