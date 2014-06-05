// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_CHARACTER_H
#define ANKI_PHYSICS_CHARACTER_H

#include "anki/Math.h"
#include <memory>

class btPairCachingGhostObject;
class btConvexShape;
class btKinematicCharacterController;
class btGhostPairCallback;

namespace anki {

class MoveComponent;
class PhysicsWorld;
class MotionState;

/// Its basically a wrapper around bullet character
class Character
{
friend class PhysicsWorld;

public:
	/// Initializer class
	struct Initializer
	{
		F32 characterHeight;
		F32 characterWidth;
		F32 stepHeight;
		F32 jumpSpeed;
		F32 maxJumpHeight;
		MoveComponent* movable; ///< For the MotionState
		Transform startTrf;

		Initializer();
	};

	Character(PhysicsWorld* masterContainer, const Initializer& init);
	~Character();
	void rotate(F32 angle);
	void moveForward(F32 distance);
	void jump();

private:
	PhysicsWorld* masterContainer; ///< Know your father
	btPairCachingGhostObject* ghostObject;
	btConvexShape* convexShape;
	btKinematicCharacterController* character;
	btGhostPairCallback* ghostPairCallback;
	std::unique_ptr<MotionState> motionState;
};

} // end namespace

#endif
