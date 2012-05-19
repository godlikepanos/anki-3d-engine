#ifndef ANKI_PHYSICS_RIGID_BODY_H
#define ANKI_PHYSICS_RIGID_BODY_H

#include "anki/math/Math.h"
#include <boost/scoped_ptr.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>


namespace anki {


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


} // end namespace


#endif
