#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include "PhyCharacter.h"
#include "Physics.h"
#include "MotionState.h"


//======================================================================================================================
// Contructor                                                                                                          =
//======================================================================================================================
PhyCharacter::PhyCharacter(Physics& physics_, const Initializer& init):
	physics(physics_)
{
	ghostObject = new btPairCachingGhostObject();

	motionState = new MotionState(toBt(init.startTrf), init.sceneNode);

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
