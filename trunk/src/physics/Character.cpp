#include "anki/physics/Character.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/physics/MotionState.h"
#include "anki/physics/RigidBody.h"
#include "anki/physics/PhysicsWorld.h"
#include <algorithm>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

namespace anki {

#if 0

//==============================================================================
// Character::Initializer                                                      = 
//==============================================================================

//==============================================================================
inline Character::Initializer::Initializer()
	: characterHeight(2.0),
		characterWidth(0.75),
		stepHeight(1.0),
		jumpSpeed(10.0),
		maxJumpHeight(0.0),
		movable(nullptr),
		startTrf(Transform::getIdentity())
{}

//==============================================================================
// Character                                                                   = 
//==============================================================================

//==============================================================================
Character::Character(PhysicsWorld* masterContainer_,
	const Initializer& init)
	: masterContainer(masterContainer_)
{
	ghostObject = new btPairCachingGhostObject();

	motionState.reset(new MotionState(init.startTrf, init.movable));

	btAxisSweep3* sweepBp =
		static_cast<btAxisSweep3*>(masterContainer->broadphase);
	ANKI_ASSERT(sweepBp != nullptr);

	ghostPairCallback = new btGhostPairCallback();
	sweepBp->getOverlappingPairCache()->setInternalGhostPairCallback(
		ghostPairCallback);
	//collisionShape = new btCapsuleShape(init.characterWidth,
	//	init.characterHeight);
	convexShape = new btCylinderShape(btVector3(init.characterWidth,
		init.characterHeight, init.characterWidth));

	ghostObject->setCollisionShape(convexShape);
	//ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	ghostObject->setWorldTransform(toBt(init.startTrf));

	character = new btKinematicCharacterController(ghostObject, convexShape,
		init.stepHeight);

	character->setJumpSpeed(init.jumpSpeed);
	character->setMaxJumpHeight(init.maxJumpHeight);

	// register
	masterContainer->dynamicsWorld->addCollisionObject(ghostObject,
		btBroadphaseProxy::CharacterFilter,
		btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);

	masterContainer->dynamicsWorld->addAction(character);

	masterContainer->characters.push_back(this);
}

//==============================================================================
Character::~Character()
{
	masterContainer->characters.erase(std::find(
		masterContainer->characters.begin(),
		masterContainer->characters.end(), this));
	masterContainer->dynamicsWorld->removeAction(character);
	masterContainer->dynamicsWorld->removeCollisionObject(ghostObject);

	delete character;
	delete convexShape;
	delete ghostPairCallback;
	delete ghostObject;
}

//==============================================================================
void Character::rotate(float angle)
{
	btMatrix3x3 rot = ghostObject->getWorldTransform().getBasis();
	rot *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), angle));
	ghostObject->getWorldTransform().setBasis(rot);
}

//==============================================================================
void Character::moveForward(float distance)
{
	btVector3 forward =
		-ghostObject->getWorldTransform().getBasis().getColumn(2);
	character->setWalkDirection(forward * distance);
}

//==============================================================================
void Character::jump()
{
	character->jump();
}

#endif

} // end namespace
