#ifndef PHYS_RIGID_BODY_H
#define PHYS_RIGID_BODY_H

#include <boost/scoped_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include "Math/Math.h"


class SceneNode;
namespace Phys {
class MotionState;
class MasterContainer;
}


namespace Phys {


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
		RigidBody(MasterContainer& masterContainer, const Initializer& init);

		/// Unregister
		~RigidBody();

	private:
		MasterContainer& masterContainer; ///< Know your father
		/// Keep it here for garbage collection
		boost::scoped_ptr<MotionState> motionState;
};


} // end namespace


#endif
