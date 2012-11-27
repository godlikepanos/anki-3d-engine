#include "anki/physics/PhysWorld.h"
#include "anki/physics/Character.h"
#include "anki/physics/MotionState.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace anki {

//==============================================================================
PhysWorld::PhysWorld()
	: defaultContactProcessingThreshold(BT_LARGE_FLOAT)
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadphase = new btAxisSweep3(
		btVector3(-1000, -1000, -1000),
		btVector3(1000, 1000, 1000));
	sol = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(
		dispatcher, broadphase, sol, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,-10, 0));
}

//==============================================================================
PhysWorld::~PhysWorld()
{
	/// XXX
}

//==============================================================================
void PhysWorld::setDebugDrawer(btIDebugDraw* newDebugDrawer)
{
	debugDrawer.reset(newDebugDrawer);
	dynamicsWorld->setDebugDrawer(debugDrawer.get());
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
}

//==============================================================================
void PhysWorld::update(F32 prevUpdateTime, F32 crntTime)
{
	dynamicsWorld->stepSimulation(crntTime - prevUpdateTime);

	// updateNonRigidBodiesMotionStates
	for(U i = 0; i < characters.size(); i++)
	{
		characters[i]->motionState->setWorldTransform(
			characters[i]->ghostObject->getWorldTransform());
	}
}

} // end namespace anki
