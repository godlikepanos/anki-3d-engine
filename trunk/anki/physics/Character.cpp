#include <algorithm>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include "anki/physics/Character.h"
#include "anki/physics/PhysWorld.h"
#include "anki/physics/MotionState.h"
#include "anki/physics/RigidBody.h"
#include "anki/physics/PhysWorld.h"


//==============================================================================
// Contructor                                                                  =
//==============================================================================
inline Character::Initializer::Initializer():
	characterHeight(2.0),
	characterWidth(0.75),
	stepHeight(1.0),
	jumpSpeed(10.0),
	maxJumpHeight(0.0),
	sceneNode(NULL),
	startTrf(Transform::getIdentity())
{}


//==============================================================================
// Contructor                                                                  =
//==============================================================================
Character::Character(PhysWorld& masterContainer_,
	const Initializer& init)
:	masterContainer(masterContainer_)
{
	ghostObject = new btPairCachingGhostObject();

	motionState = new MotionState(init.startTrf, init.sceneNode);

	btAxisSweep3* sweepBp =
		dynamic_cast<btAxisSweep3*>(masterContainer.broadphase);
	ASSERT(sweepBp != NULL);

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
	masterContainer.dynamicsWorld->addCollisionObject(ghostObject,
		btBroadphaseProxy::CharacterFilter,
		btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);

	masterContainer.dynamicsWorld->addAction(character);

	masterContainer.characters.push_back(this);
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Character::~Character()
{
	masterContainer.characters.erase(std::find(
		masterContainer.characters.begin(),
		masterContainer.characters.end(), this));
	masterContainer.dynamicsWorld->removeAction(character);
	masterContainer.dynamicsWorld->removeCollisionObject(ghostObject);

	delete character;
	delete convexShape;
	delete ghostPairCallback;
	delete ghostObject;
}


//==============================================================================
// rotate                                                                      =
//==============================================================================
void Character::rotate(float angle)
{
	btMatrix3x3 rot = ghostObject->getWorldTransform().getBasis();
	rot *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), angle));
	ghostObject->getWorldTransform().setBasis(rot);
}


//==============================================================================
// moveForward                                                                 =
//==============================================================================
void Character::moveForward(float distance)
{
	btVector3 forward =
		-ghostObject->getWorldTransform().getBasis().getColumn(2);
	character->setWalkDirection(forward * distance);
}


//==============================================================================
// jump                                                                        =
//==============================================================================
void Character::jump()
{
	character->jump();
}
