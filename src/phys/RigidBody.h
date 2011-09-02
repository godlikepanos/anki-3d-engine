#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <boost/scoped_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include "m/Math.h"


class SceneNode;
class MotionState;
class PhysWorld;


/// Wrapper for rigid body
class RigidBody: public btRigidBody
{
	public:
		/// Initializer class
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

		/// Init and register
		RigidBody(PhysWorld& masterContainer, const Initializer& init);

		/// Unregister
		~RigidBody();

	private:
		PhysWorld& masterContainer; ///< Know your father
		/// Keep it here for garbage collection
		boost::scoped_ptr<MotionState> motionState;
};


#endif
