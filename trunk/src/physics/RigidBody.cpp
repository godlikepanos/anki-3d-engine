#include "anki/physics/RigidBody.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/MoveComponent.h"

namespace anki {

//==============================================================================
RigidBody::RigidBody(PhysicsWorld* world, const Initializer& init)
	:	PhysicsObject(world),
		btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0.0, nullptr, 
			nullptr, btVector3(0.0, 0.0, 0.0))), // dummy init
		SceneComponent(this)
{
	ANKI_ASSERT(init.shape != nullptr 
		&& init.shape->getShapeType() != INVALID_SHAPE_PROXYTYPE);

	Bool isDynamic = (init.mass != 0.0);

	btVector3 localInertia;
	if(isDynamic)
	{
		init.shape->calculateLocalInertia(init.mass, localInertia);
	}
	else
	{
		localInertia = btVector3(0.0, 0.0, 0.0);
	}

	motionState = MotionState(toBt(init.startTrf));

	btRigidBody::btRigidBodyConstructionInfo cInfo(
		init.mass, &motionState, init.shape, localInertia);

	setupRigidBody(cInfo);

	setContactProcessingThreshold(
		getPhysicsWorld().defaultContactProcessingThreshold);

	if(init.kinematic)
	{
		// XXX
	}

	//forceActivationState(ISLAND_SLEEPING);

	// register
	if(init.mask == -1 || init.group == -1)
	{
		getPhysicsWorld().dynamicsWorld->addRigidBody(this);
	}
	else
	{
		getPhysicsWorld().dynamicsWorld->addRigidBody(
			this, init.group, init.mask);
	}
}

//==============================================================================
RigidBody::~RigidBody()
{
	getPhysicsWorld().dynamicsWorld->removeRigidBody(this);
}

//==============================================================================
Bool RigidBody::syncUpdate(SceneNode& node, F32 prevTime, F32 crntTime)
{
	MoveComponent* move = node.getMoveComponent();
	if(ANKI_UNLIKELY(move == nullptr))
	{
		return false;
	}

	Bool moveUpdated = move->bitsEnabled(MoveComponent::MF_MARKED_FOR_UPDATE);
	Transform oldTrf = move->getLocalTransform();

	Bool mstateUpdated = motionState.getUpdated();

	// Sync from motion state to move component
	if(mstateUpdated)
	{
		// Set local transform and preserve scale
		Transform newTrf;
		F32 originalScale = move->getLocalTransform().getScale();
		newTrf = toAnki(motionState.getWorldTransform2());
		newTrf.setScale(originalScale);
		std::cout << newTrf.getOrigin().toString() << std::endl;
		move->setLocalTransform(newTrf);

		// Set the flag
		motionState.setUpdated(false);

		std::cout << "From motion state to move component" << std::endl;
	}

	// Sync from move component to motion state
	if(moveUpdated)
	{
		std::cout << "Activating again " << oldTrf.getOrigin().toString() << std::endl;

		setWorldTransform(toBt(oldTrf));
		//motionState.setWorldTransform(toBt(oldTrf));
		//forceActivationState(ACTIVE_TAG);
		activate();
		//clearForces();
		//setLinearVelocity(btVector3(0.0, 0.0, 0.0));
		//setAngularVelocity(btVector3(0.0, 0.0, 0.0));

		//setGravity(getPhysicsWorld().dynamicsWorld->getGravity());
	}

	return mstateUpdated;
}

} // end namespace anki
