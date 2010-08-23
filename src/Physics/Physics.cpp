#include "Physics.h"
#include "PhyCharacter.h"



//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Physics::Physics():
	defaultContactProcessingThreshold(BT_LARGE_FLOAT),
	time(0.0)
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
// update                                                                                                              =
//======================================================================================================================
void Physics::update(float crntTime)
{
	float dt = crntTime - time;
	time = crntTime;

	dynamicsWorld->stepSimulation(dt);

	// updateNonRigidBodiesMotionStates
	for(uint i=0; i<characters.size(); i++)
	{
		characters[i]->motionState->setWorldTransform(characters[i]->ghostObject->getWorldTransform());
	}
}
