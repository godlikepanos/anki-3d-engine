#ifndef _PHYWORLD_H_
#define _PHYWORLD_H_

#include "Common.h"
#include "PhyCommon.h"

/**
 *
 */
class PhyWorld
{
	PROPERTY_R( btDiscreteDynamicsWorld*, dynamicsWorld, getDynamicsWorld )

	private:
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btDbvtBroadphase* broadphase;
		btSequentialImpulseConstraintSolver* sol;

	public:
		/**
		 * Collision groups
		 */
		enum
		{
			CG_NOTHING = 0,
			CG_MAP = 1,
			CG_PARTICLE = 2
		};


		PhyWorld()
		{
			collisionConfiguration = new btDefaultCollisionConfiguration();
			dispatcher = new	btCollisionDispatcher(collisionConfiguration);
			broadphase = new btDbvtBroadphase();
			sol = new btSequentialImpulseConstraintSolver;
			dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, broadphase, sol, collisionConfiguration );
			dynamicsWorld->setGravity(btVector3(0,-10,0));
		}
};

#endif
