#ifndef _PHYWORLD_H_
#define _PHYWORLD_H_

#include "Common.h"
#include "PhyCommon.h"

/**
 *
 */
class PhyWorld
{
	PROPERTY_R(btDiscreteDynamicsWorld*, dynamicsWorld, getDynamicsWorld)
	PROPERTY_R(float, defaultContactProcessingThreshold, getDefaultContactProcessingThreshold)

	public:
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btDbvtBroadphase* broadphase;
		btSequentialImpulseConstraintSolver* sol;

		/**
		 * Collision groups
		 */
		enum
		{
			CG_NOTHING = 0,
			CG_MAP = 1,
			CG_PARTICLE = 2
		};


		PhyWorld() :
			defaultContactProcessingThreshold(BT_LARGE_FLOAT)
		{
			collisionConfiguration = new btDefaultCollisionConfiguration();
			dispatcher = new	btCollisionDispatcher(collisionConfiguration);
			broadphase = new btDbvtBroadphase();
			sol = new btSequentialImpulseConstraintSolver;
			dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, sol, collisionConfiguration);
			dynamicsWorld->setGravity(btVector3(0,-10, 0));
		}

		/**
		 * Creates a new rigid body and adds it to the @ref dynamicsWorld. It allocates memory so the caller is responsible for cleaning up
		 * @param mass The mass of the rigid body. Put zero for static unmovable objects
		 * @param startTransform The initial position and orientation
		 * @param shape The collision shape
		 * @param node The scene node the body moves
		 * @param group The group of the body. Leave it blank if there is no group
		 * @param mask The mask of the body. Leave it blank if there is no mask
		 * @return A new rigid body
		 */
		btRigidBody* createNewRigidBody(float mass, const Transform& startTransform, btCollisionShape* shape, SceneNode* node, int group=-1,
		                                 int mask=-1);
};

#endif
