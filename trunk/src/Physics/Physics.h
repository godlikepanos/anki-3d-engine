#ifndef PHYSICS_H
#define PHYSICS_H

#include "Common.h"
#include "PhyCommon.h"
#include "BtAndAnkiConvertors.h"
#include "DebugDrawer.h"


class PhyCharacter;


/**
 * The master container for all physics related stuff.
 */
class Physics
{
	friend class PhyCharacter; ///< For registering and unregistering

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
		void update(float crntTime);

	private:
		Vec<PhyCharacter*> characters;
		float time; ///< Time of prev update
};

#endif
