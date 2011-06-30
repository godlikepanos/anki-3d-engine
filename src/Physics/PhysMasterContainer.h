#ifndef PHYS_MASTER_CONTAINER_H
#define PHYS_MASTER_CONTAINER_H

#include "PhysConvertors.h"
#include "Util/Vec.h"
#include <boost/scoped_ptr.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>


class btIDebugDraw;
namespace Phys {
class Character;
class RigidBody;
}


namespace Phys {


/// The master container for all physics related stuff.
class MasterContainer
{
	friend class Character; ///< For registering and unregistering
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
		~MasterContainer();

		/// @name Accessors
		/// @{
		btDiscreteDynamicsWorld& getWorld() {return *dynamicsWorld;}
		void setDebugDrawer(btIDebugDraw* newDebugDrawer);
		/// @}

		/// Time as always in sec
		void update(float prevUpdateTime, float crntTime);

	private:
		btDiscreteDynamicsWorld* dynamicsWorld; ///< Container for rigid bodied and constraints
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher; ///< Contains the algorithms of collision
		btBroadphaseInterface* broadphase;
		btSequentialImpulseConstraintSolver* sol;
		boost::scoped_ptr<btIDebugDraw> debugDrawer; ///< Keep here for garbage collection
		float defaultContactProcessingThreshold;
		Vec<Character*> characters;
};


} // end namespace

#endif
