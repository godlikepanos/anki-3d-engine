#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include "PhyCharacter.h"
#include "Physics.h"


//======================================================================================================================
// Contructor                                                                                                          =
//======================================================================================================================
PhyCharacter::PhyCharacter(Physics& physics, const Initializer& init)
{
	ghostObject = new btPairCachingGhostObject();

	btAxisSweep3* sweepBp = dynamic_cast<btAxisSweep3*>(physics.broadphase);
	DEBUG_ERR(sweepBp == NULL);

	ghostPairCallback = new btGhostPairCallback();
	sweepBp->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback);
	//collisionShape = new btCapsuleShape(init.characterWidth, init.characterHeight);
	convexShape = new btCylinderShape(btVector3(init.characterWidth, init.characterHeight, init.characterWidth));

	ghostObject->setCollisionShape(convexShape);
	//ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	btTransform trf;
	trf.setIdentity();
	trf.setOrigin(btVector3(0.0, 40.0, 0.0));
	ghostObject->setWorldTransform(trf);

	character = new btKinematicCharacterController(ghostObject, convexShape, init.stepHeight);

	character->setJumpSpeed(init.jumpSpeed);
	character->setMaxJumpHeight(init.maxJumpHeight);

	// register
	physics.dynamicsWorld->addCollisionObject(ghostObject, btBroadphaseProxy::CharacterFilter,
																						btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);

	physics.dynamicsWorld->addAction(character);
}
