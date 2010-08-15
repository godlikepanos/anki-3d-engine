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
};

#endif
