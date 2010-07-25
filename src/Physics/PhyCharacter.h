#ifndef _PHYCHARACTER_H_
#define _PHYCHARACTER_H_

#include "Common.h"
#include "PhyCommon.h"


/**
 * @todo write docs
 */
class PhyCharacter
{
	public:
		btPairCachingGhostObject* ghostObject;
		btCapsuleShape* capsule;
		btKinematicCharacterController* character;

		PhyCharacter(Physics* world, float charHeight, float charWidth, float stepHeight, float maxJumpHeight)
		{
			ghostObject = new btPairCachingGhostObject();
			world->broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
			capsule = new btCapsuleShape(charWidth, charHeight);
			ghostObject->setCollisionShape(capsule);
			ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
			character = new btKinematicCharacterController(ghostObject, capsule, stepHeight);
			character->setMaxJumpHeight(maxJumpHeight);
		}


};

#endif
