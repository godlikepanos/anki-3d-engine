#include <algorithm>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include "PhysCharacter.h"
#include "PhysMasterContainer.h"
#include "PhysMotionState.h"
#include "PhysRigidBody.h"


namespace Phys {


//======================================================================================================================
// Contructor                                                                                                          =
//======================================================================================================================
inline Character::Initializer::Initializer():
	characterHeight(2.0),
	characterWidth(0.75),
	stepHeight(1.0),
	jumpSpeed(10.0),
	maxJumpHeight(0.0),
	sceneNode(NULL),
	startTrf(Transform::getIdentity())
{}


//======================================================================================================================
// Contructor                                                                                                          =
//======================================================================================================================
Character::Character(MasterContainer& masterContainer_, const Initializer& init, Object* parent):
	Object(parent),
	masterContainer(masterContainer_)
{
	ghostObject = new btPairCachingGhostObject();

	motionState = new MotionState(init.startTrf, init.sceneNode);

	btAxisSweep3* sweepBp = dynamic_cast<btAxisSweep3*>(physics.broadphase);
	ASSERT(sweepBp != NULL);

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
Character::~Character()
{
	physics.characters.erase(std::find(physics.characters.begin(), physics.characters.end(), this));
	physics.dynamicsWorld->removeAction(character);
	physics.dynamicsWorld->removeCollisionObject(ghostObject);

	delete character;
	delete convexShape;
	delete ghostPairCallback;
	delete ghostObject;
}


//======================================================================================================================
// rotate                                                                                                              =
//======================================================================================================================
void Character::rotate(float angle)
{
	btMatrix3x3 rot = ghostObject->getWorldTransform().getBasis();
	rot *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), angle));
	ghostObject->getWorldTransform().setBasis(rot);
}


//======================================================================================================================
// moveForward                                                                                                         =
//======================================================================================================================
void Character::moveForward(float distance)
{
	btVector3 forward = -ghostObject->getWorldTransform().getBasis().getColumn(2);
	character->setWalkDirection(forward * distance);
}


//======================================================================================================================
// jump                                                                                                                =
//======================================================================================================================
void Character::jump()
{
	character->jump();
}


} // end namespace
