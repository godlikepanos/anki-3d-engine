#include "anki/physics/PhysicsWorld.h"
#include "anki/scene/SceneGraph.h"
#include "anki/physics/Character.h"
#include "anki/physics/MotionState.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace anki {

//==============================================================================
PhysicsWorld::PhysicsWorld(SceneGraph* scene_)
	:	scene(scene_),
		defaultContactProcessingThreshold(BT_LARGE_FLOAT),
		characters(getSceneAllocator())

{
	SceneAllocator<U8> alloc = getSceneAllocator();

	collisionConfiguration = 
		alloc.newInstance<btDefaultCollisionConfiguration>();

	dispatcher = alloc.newInstance<btCollisionDispatcher>(
		collisionConfiguration);

	broadphase = alloc.newInstance<btAxisSweep3>(
		btVector3(-1000, -1000, -1000),
		btVector3(1000, 1000, 1000));

	sol = alloc.newInstance<btSequentialImpulseConstraintSolver>();

	dynamicsWorld = alloc.newInstance<btDiscreteDynamicsWorld>(
		dispatcher, broadphase, sol, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

//==============================================================================
PhysicsWorld::~PhysicsWorld()
{
	SceneAllocator<U8> alloc = getSceneAllocator();

	alloc.deleteInstance(dynamicsWorld);
	alloc.deleteInstance(sol);
	alloc.deleteInstance(broadphase);
	alloc.deleteInstance(dispatcher);
	alloc.deleteInstance(collisionConfiguration);
}

//==============================================================================
SceneAllocator<U8> PhysicsWorld::getSceneAllocator() const
{
	return scene->getAllocator();
}

//==============================================================================
void PhysicsWorld::setDebugDrawer(btIDebugDraw* newDebugDrawer)
{
	debugDrawer.reset(newDebugDrawer);
	dynamicsWorld->setDebugDrawer(debugDrawer.get());
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
}

//==============================================================================
void PhysicsWorld::update(F32 prevUpdateTime, F32 crntTime)
{
	F32 dt = crntTime - prevUpdateTime;
	dynamicsWorld->stepSimulation(dt);

	// updateNonRigidBodiesMotionStates
	for(U i = 0; i < characters.size(); i++)
	{
		characters[i]->motionState->setWorldTransform(
			characters[i]->ghostObject->getWorldTransform());
	}
}

//==============================================================================
void PhysicsWorld::debugDraw()
{
	dynamicsWorld->debugDrawWorld();
}

} // end namespace anki
