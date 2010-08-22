#ifndef PHYCHARACTER_H
#define PHYCHARACTER_H

#include "Common.h"
#include "Physics.h"


class Physics;
class btPairCachingGhostObject;
class btConvexShape;
class btKinematicCharacterController;
class btGhostPairCallback;


/**
 * Its basically a wrapper around bullet character
 */
class PhyCharacter
{
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

			Initializer();
		};

		PhyCharacter(Physics& physics, const Initializer& init);
		~PhyCharacter();
		void rotate(float angle);
		void moveForward(float distance);
		void jump();

	private:
		btPairCachingGhostObject* ghostObject;
		btConvexShape* convexShape;
		btKinematicCharacterController* character;
		btGhostPairCallback* ghostPairCallback;
};


inline PhyCharacter::Initializer::Initializer():
	characterHeight(2.0),
	characterWidth(0.75),
	stepHeight(1.0),
	jumpSpeed(10.0),
	maxJumpHeight(.0)
{}

#endif
