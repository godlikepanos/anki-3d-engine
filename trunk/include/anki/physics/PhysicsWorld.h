#ifndef ANKI_PHYSICS_PHYS_WORLD_H
#define ANKI_PHYSICS_PHYS_WORLD_H

#include "anki/physics/Converters.h"
#include "anki/physics/PhysicsObject.h"
#include "anki/util/Vector.h"
#include "anki/scene/Common.h"
#include <memory>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

class btIDebugDraw;

namespace anki {

class Character;
class RigidBody;
class SceneGraph;

/// The master container for all physics related stuff.
class PhysicsWorld
{
	// Friends for registering and unregistering
	friend class Character; 
	friend class RigidBody;

public:
	/// Collision groups
	enum CollisionGroup
	{
		CG_NOTHING = 0,
		CG_MAP = 1 << 0,
		CG_PARTICLE = 1 << 1,
		CG_ALL = CG_MAP | CG_PARTICLE
	};

	PhysicsWorld(SceneGraph* scene);
	~PhysicsWorld();

	SceneAllocator<U8> getSceneAllocator() const;

	/// @note This class takes ownership of the pointer
	void setDebugDrawer(btIDebugDraw* newDebugDrawer);

	void debugDraw();

	/// Time as always in sec
	void update(F32 prevUpdateTime, F32 crntTime);

	/// Create a new SceneNode
	template<typename Object, typename... Args>
	void newPhysicsObject(Object*& object, Args&&... args)
	{
		SceneAllocator<Object> al = getSceneAllocator();
		//object = al.newInstance(this, std::forward<Args>(args)...);
		object = al.allocate(1);
		al.construct(object, this, std::forward<Args>(args)...);
	}

	/// Delete a scene node. It actualy marks it for deletion
	void deletePhysicsObject(PhysicsObject* object)
	{
		SceneAllocator<PhysicsObject> al = getSceneAllocator();
		al.deleteInstance(object);
	}

private:
	SceneGraph* scene;

	/// Container for rigid bodied and constraints
	btDiscreteDynamicsWorld* dynamicsWorld;
	btDefaultCollisionConfiguration* collisionConfiguration;
	/// Contains the algorithms of collision
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* broadphase;
	btSequentialImpulseConstraintSolver* sol;
	/// Keep here for garbage collection
	std::unique_ptr<btIDebugDraw> debugDrawer;
	F32 defaultContactProcessingThreshold;

	SceneVector<Character*> characters;
};

} // end namespace anki

#endif
