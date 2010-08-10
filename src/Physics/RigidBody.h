#ifndef MY_RIGIDBODY_H // Watch the naming cause Bullet uses the same
#define MY_RIGIDBODY_H

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include "Common.h"
#include "Object.h"
#include "Math.h"


class SceneNode;


/**
 * Wrapper for rigid body
 */
class RigidBody: public btRigidBody, public Object
{
	public:
		/**
		 *
		 * @param mass
		 * @param startTransform
		 * @param shape
		 * @param node
		 * @param group
		 * @param mask
		 * @param parent
		 * @return
		 */
		RigidBody(float mass, const Transform& startTransform, btCollisionShape* shape, SceneNode* node,
		          int group = -1, int mask = -1, Object* parent = NULL);
};


#endif
