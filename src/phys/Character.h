#ifndef CHARACTER_H
#define CHARACTER_H

#include "m/Math.h"
#include "core/Object.h"


class btPairCachingGhostObject;
class btConvexShape;
class btKinematicCharacterController;
class btGhostPairCallback;
class SceneNode;
class PhysWorld;
class MotionState;


/// Its basically a wrapper around bullet character
class Character
{
	friend class PhysWorld;

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

		Character(PhysWorld& masterContainer, const Initializer& init);
		~Character();
		void rotate(float angle);
		void moveForward(float distance);
		void jump();

	private:
		PhysWorld& masterContainer; ///< Know your father
		btPairCachingGhostObject* ghostObject;
		btConvexShape* convexShape;
		btKinematicCharacterController* character;
		btGhostPairCallback* ghostPairCallback;
		MotionState* motionState;
};


#endif
