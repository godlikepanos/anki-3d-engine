#ifndef PHYS_CHARACTER_H
#define PHYS_CHARACTER_H

#include "PhysMasterContainer.h"
#include "Math.h"
#include "Object.h"


class btPairCachingGhostObject;
class btConvexShape;
class btKinematicCharacterController;
class btGhostPairCallback;
class SceneNode;
namespace Phys {
class MasterContainer;
class MotionState;
}


namespace Phys {


/// Its basically a wrapper around bullet character
class Character
{
	friend class MasterContainer;

	public:
		/// Initializer class
		struct Initializer
		{
			float characterHeight;
			float characterWidth;
			float stepHeight;
			float jumpSpeed;
			float maxJumpHeight;
			SceneNode* sceneNode; ///< For the MotionState
			Transform startTrf;

			Initializer();
		};

		Character(MasterContainer& masterContainer_, const Initializer& init);
		~Character();
		void rotate(float angle);
		void moveForward(float distance);
		void jump();

	private:
		MasterContainer& masterContainer; ///< Know your father
		btPairCachingGhostObject* ghostObject;
		btConvexShape* convexShape;
		btKinematicCharacterController* character;
		btGhostPairCallback* ghostPairCallback;
		MotionState* motionState;
};


} // end namespace


#endif
