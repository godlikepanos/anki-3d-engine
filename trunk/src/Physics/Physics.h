#ifndef PHYSICS_H
#define PHYSICS_H

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "Object.h"
#include "BtAndAnkiConvertors.h"
#include "DebugDrawer.h"


class PhyCharacter;
class RigidBody;


/**
 * The master container for all physics related stuff.
 */
class Physics: public Object
{
	friend class PhyCharacter; ///< For registering and unregistering
	friend class RigidBody;  ///< For registering and unregistering

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

	public:
		Physics(Object* parent);
		void update(float crntTime);
		void debugDraw();

		/**
		 * @name Accessors
		 */
		/**@{*/
		btDiscreteDynamicsWorld& getWorld() {return *dynamicsWorld;}
		/**@}*/

	private:
		btDiscreteDynamicsWorld* dynamicsWorld;
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btBroadphaseInterface* broadphase;
		btSequentialImpulseConstraintSolver* sol;
		DebugDrawer* debugDrawer;
		float defaultContactProcessingThreshold;
		Vec<PhyCharacter*> characters;
		float time; ///< Time of prev update
};


inline void Physics::debugDraw()
{
	dynamicsWorld->debugDrawWorld();
}

#endif
