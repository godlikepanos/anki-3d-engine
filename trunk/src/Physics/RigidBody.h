#ifndef MY_RIGIDBODY_H // Watch the naming cause Bullet uses the same
#define MY_RIGIDBODY_H

#include <memory>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include "Common.h"
#include "Math.h"


class SceneNode;
class MotionState;


/**
 * Wrapper for rigid body
 */
class RigidBody: public btRigidBody
{
	public:
		/**
		 * Init and register
		 * @param mass S/E
		 * @param startTransform Initial pos
		 * @param shape Collision shape
		 * @param node SceneNode to move
		 * @param group -1 if not used
		 * @param mask -1 if not used
		 */
		RigidBody(float mass, const Transform& startTransform, btCollisionShape* shape, SceneNode* node,
		          int group = -1, int mask = -1);

		/**
		 * Unregister it
		 */
		~RigidBody();

	private:
		auto_ptr<MotionState> motionState; ///< Keep it here as well for garbage collection
};


#endif
