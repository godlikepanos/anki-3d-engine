#ifndef PHYS_WORLD_H
#define PHYS_WORLD_H

#include "Convertors.h"
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>


class btIDebugDraw;
class Character;
class RigidBody;


/// The master container for all physics related stuff.
class PhysWorld
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
		PhysWorld();
		~PhysWorld();

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
		std::vector<Character*> characters;
};


#endif
