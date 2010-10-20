#ifndef MY_RIGIDBODY_H // Watch the naming cause Bullet uses the same
#define MY_RIGIDBODY_H

#include <memory>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include "Math.h"
#include "Object.h"


class SceneNode;
class MotionState;
class Physics;


/**
 * Wrapper for rigid body
 */
class RigidBody: public btRigidBody, public Object
{
	public:
		/**
		 * Initializer class
		 */
		struct Initializer
		{
			float mass;
			Transform startTrf;
			btCollisionShape* shape;
			SceneNode* sceneNode;
		  int group;
		  int mask;

		  Initializer();
		};

		/**
		 * Init and register
		 */
		RigidBody(Physics& physics, const Initializer& init, Object* parent = NULL);

		/**
		 * Unregister
		 */
		~RigidBody();

	private:
		Physics& physics; ///< Know your father
		MotionState* motionState; ///< Keep it here as well for garbage collection
};


inline RigidBody::Initializer::Initializer():
	mass(0.0),
	startTrf(Transform::getIdentity()),
	shape(NULL),
	sceneNode(NULL),
	group(-1),
	mask(-1)
{}

#endif
