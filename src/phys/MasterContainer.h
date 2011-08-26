#ifndef PHYS_MASTER_CONTAINER_H
#define PHYS_MASTER_CONTAINER_H

#include "Convertors.h"
#include "util/Vec.h"
#include <boost/scoped_ptr.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>


class btIDebugDraw;
namespace phys {
class Character;
class RigidBody;
}


namespace phys {


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
		/// Container for rigid bodied and constraints
		btDiscreteDynamicsWorld* dynamicsWorld;
		btDefaultCollisionConfiguration* collisionConfiguration;
		/// Contains the algorithms of collision
		btCollisionDispatcher* dispatcher;
		btBroadphaseInterface* broadphase;
		btSequentialImpulseConstraintSolver* sol;
		/// Keep here for garbage collection
		boost::scoped_ptr<btIDebugDraw> debugDrawer;
		float defaultContactProcessingThreshold;
		Vec<Character*> characters;
};


} // end namespace

#endif
