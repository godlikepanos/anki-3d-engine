#ifndef PHYS_MASTER_CONTAINER_H
#define PHYS_MASTER_CONTAINER_H

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BtAndAnkiConvertors.h"
#include "PhyDbgDrawer.h" ///< @todo Remove this crap from here. Its Renderer's stuff
#include "Vec.h"


namespace Phys {
class PhyCharacter;
class RigidBody;
}


namespace Phys {


/// The master container for all physics related stuff.
class MasterContainer
{
	friend class PhyCharacter; ///< For registering and unregistering
	friend class RigidBody;  ///< For registering and unregistering

	public:
		/// Collision groups
		enum CollisionGroup
		{
			CG_NOTHING = 0,
			CG_MAP = 1,
			CG_PARTICLE = 2,
			CG_ALL = CG_MAP | CG_PARTICLE
		};

	public:
		MasterContainer();

		/// @name Accessors
		/// @{
		btDiscreteDynamicsWorld& getWorld() {return *dynamicsWorld;}
		/// @}

		/// Time as always in ms
		void update(uint prevUpdateTime, uint crntTime);

	private:
		btDiscreteDynamicsWorld* dynamicsWorld;
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btBroadphaseInterface* broadphase;
		btSequentialImpulseConstraintSolver* sol;
		PhyDbgDrawer* debugDrawer; ///< @todo Remove this crap from here. Its Renderer's stuff
		float defaultContactProcessingThreshold;
		Vec<PhyCharacter*> characters;
};


} // end namespace

#endif
