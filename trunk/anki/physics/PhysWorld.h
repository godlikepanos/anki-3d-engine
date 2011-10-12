#ifndef ANKI_PHYSICS_PHYS_WORLD_H
#define ANKI_PHYSICS_PHYS_WORLD_H

#include "anki/physics/Convertors.h"
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>


class btIDebugDraw;


namespace anki {


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


} // end namespace


#endif
