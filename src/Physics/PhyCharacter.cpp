#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include "PhyCharacter.h"
#include "Physics.h"
#include "MotionState.h"
#include "RigidBody.h"


//======================================================================================================================
// Contructor                                                                                                          =
//======================================================================================================================
PhyCharacter::PhyCharacter(Physics& physics_, const Initializer& init):
	physics(physics_)
{
	ghostObject = new btPairCachingGhostObject();

	motionState = new MotionState(init.startTrf, init.sceneNode);

	btAxisSweep3* sweepBp = dynamic_cast<btAxisSweep3*>(physics.broadphase);
	DEBUG_ERR(sweepBp == NULL);

	ghostPairCallback = new btGhostPairCallback();
	sweepBp->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback);
	//collisionShape = new btCapsuleShape(init.characterWidth, init.characterHeight);
	convexShape = new btCylinderShape(btVector3(init.characterWidth, init.characterHeight, init.characterWidth));

	ghostObject->setCollisionShape(convexShape);
	//ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	ghostObject->setWorldTransform(toBt(init.startTrf));

	character = new btKinematicCharacterController(ghostObject, convexShape, init.stepHeight);

	character->setJumpSpeed(init.jumpSpeed);
	character->setMaxJumpHeight(init.maxJumpHeight);

	// register
	physics.dynamicsWorld->addCollisionObject(ghostObject, btBroadphaseProxy::CharacterFilter,
																						btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);

	physics.dynamicsWorld->addAction(character);

	physics.characters.push_back(this);
}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
PhyCharacter::~PhyCharacter()
{
	physics.characters.erase(std::find(physics.characters.begin(), physics.characters.end(), this));
	physics.dynamicsWorld->removeAction(character);
	physics.dynamicsWorld->removeCollisionObject(ghostObject);

	delete character;
	delete convexShape;
	delete ghostPairCallback;
	delete ghostObject;
	delete motionState;
}


//======================================================================================================================
// rotate                                                                                                              =
//======================================================================================================================
void PhyCharacter::rotate(float angle)
{
	btMatrix3x3 rot = ghostObject->getWorldTransform().getBasis();
	rot *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), angle));
	ghostObject->getWorldTransform().setBasis(rot);
}


//======================================================================================================================
// moveForward                                                                                                         =
//======================================================================================================================
void PhyCharacter::moveForward(float distance)
{
	btVector3 forward = -ghostObject->getWorldTransform().getBasis().getColumn(2);
	character->setWalkDirection(forward * distance);
}


//======================================================================================================================
// jump                                                                                                                =
//======================================================================================================================
void PhyCharacter::jump()
{
	character->jump();
}
