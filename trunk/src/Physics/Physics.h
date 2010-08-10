#ifndef PHYSICS_H
#define PHYSICS_H

#include "Common.h"
#include "PhyCommon.h"
#include "BtAndAnkiConvertors.h"
#include "DebugDrawer.h"


/**
 * The master container for all physics related stuff.
 */
class Physics
{
	public:
		/**
		 * Collision groups
		 */
		enum CollisionGroup
		{
			CG_NOTHING = 0,
			CG_MAP = 1,
			CG_PARTICLE = 2,
			CG_ALL = CG_MAP | CG_PARTICLE
		};

	PROPERTY_R(btDiscreteDynamicsWorld*, dynamicsWorld, getDynamicsWorld)
	PROPERTY_R(float, defaultContactProcessingThreshold, getDefaultContactProcessingThreshold)

	public:
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btDbvtBroadphase* broadphase;
		btSequentialImpulseConstraintSolver* sol;
		DebugDrawer* debugDrawer;

		Physics();

		/**
		 * Creates a new rigid body and adds it to the @ref dynamicsWorld. It allocates memory so the caller is responsible
		 * for cleaning up
		 * @param mass The mass of the rigid body. Put zero for static unmovable objects
		 * @param startTransform The initial position and orientation
		 * @param shape The collision shape
		 * @param node The scene node the body moves
		 * @param group The group of the body. Leave it blank if there is no group
		 * @param mask The mask of the body. Leave it blank if there is no mask
		 * @return A new rigid body
		 */
		btRigidBody* createNewRigidBody(float mass, const Transform& startTransform, btCollisionShape* shape,
		                                SceneNode* node, int group = -1, int mask = -1);

		/**
		 * Removes the body from the world, deletes its motion state and deletes it
		 */
		void deleteRigidBody(btRigidBody* body);
};

#endif
