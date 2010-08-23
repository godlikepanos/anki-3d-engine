#ifndef PHYCHARACTER_H
#define PHYCHARACTER_H

#include "Common.h"
#include "Physics.h"
#include "Math.h"


class Physics;
class btPairCachingGhostObject;
class btConvexShape;
class btKinematicCharacterController;
class btGhostPairCallback;
class MotionState;


/**
 * Its basically a wrapper around bullet character
 */
class PhyCharacter
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
			SceneNode* sceneNode;
			Transform startTrf;

			Initializer();
		};

		PhyCharacter(Physics& physics_, const Initializer& init);
		~PhyCharacter();
		void rotate(float angle);
		void moveForward(float distance);
		void jump();

	private:
		Physics& physics;
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
