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
	private:
		auto_ptr<MotionState> motionState; ///< Keep it here as well for garbage collection

	public:
		/**
		 * Create and register
		 * @param mass
		 * @param startTransform Initial pos
		 * @param shape
		 * @param node
		 * @param group -1 if not used
		 * @param mask -1 if not used
		 * @param parent Object stuff
		 */
		RigidBody(float mass, const Transform& startTransform, btCollisionShape* shape, SceneNode* node,
		          int group = -1, int mask = -1);

		/**
		 * Unregister it
		 */
		~RigidBody();
};


#endif
