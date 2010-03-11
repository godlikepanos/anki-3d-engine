#ifndef _PHYWORLD_H_
#define _PHYWORLD_H_

#include "Common.h"
#include "PhyCommon.h"

/**
 *
 */
class PhyWorld
{
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


		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btDbvtBroadphase* broadphase;
		btSequentialImpulseConstraintSolver* sol;
		btDiscreteDynamicsWorld* dynamicsWorld;
};

#endif
