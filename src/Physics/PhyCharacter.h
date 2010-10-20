#ifndef PHY_CHARACTER_H
#define PHY_CHARACTER_H

#include "Physics.h"
#include "Math.h"
#include "Object.h"


class Physics;
class btPairCachingGhostObject;
class btConvexShape;
class btKinematicCharacterController;
class btGhostPairCallback;
class MotionState;
class SceneNode;


/**
 * Its basically a wrapper around bullet character
 */
class PhyCharacter: public Object
{
	friend class Physics;

	public:
		/**
		 * Initializer class
		 */
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

		PhyCharacter(Physics& physics_, const Initializer& init, Object* parent = NULL);
		~PhyCharacter();
		void rotate(float angle);
		void moveForward(float distance);
		void jump();

	private:
		Physics& physics; ///< Know your father
		btPairCachingGhostObject* ghostObject;
		btConvexShape* convexShape;
		btKinematicCharacterController* character;
		btGhostPairCallback* ghostPairCallback;
		MotionState* motionState;
};


inline PhyCharacter::Initializer::Initializer():
	characterHeight(2.0),
	characterWidth(0.75),
	stepHeight(1.0),
	jumpSpeed(10.0),
	maxJumpHeight(0.0),
	sceneNode(NULL),
	startTrf(Transform::getIdentity())
{}


#endif
