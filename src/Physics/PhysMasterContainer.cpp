#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "PhysMasterContainer.h"
#include "PhysCharacter.h"
#include "PhysMotionState.h"


namespace Phys {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MasterContainer::MasterContainer():
	defaultContactProcessingThreshold(BT_LARGE_FLOAT)
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadphase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
	sol = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, sol, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,-10, 0));
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
MasterContainer::~MasterContainer()
{
	/// @todo
}


//==============================================================================
// setDebugDrawer                                                              =
//==============================================================================
void MasterContainer::setDebugDrawer(btIDebugDraw* newDebugDrawer)
{
	debugDrawer.reset(newDebugDrawer);
	dynamicsWorld->setDebugDrawer(debugDrawer.get());
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
}


//==============================================================================
// update                                                                      =
//==============================================================================
void MasterContainer::update(float prevUpdateTime, float crntTime)
{
	dynamicsWorld->stepSimulation(crntTime - prevUpdateTime);

	// updateNonRigidBodiesMotionStates
	for(uint i = 0; i < characters.size(); i++)
	{
		characters[i]->motionState->setWorldTransform(characters[i]->ghostObject->getWorldTransform());
	}
}


} // end namespace
