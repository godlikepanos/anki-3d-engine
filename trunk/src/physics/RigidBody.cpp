// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

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
		SceneComponent(RIGID_BODY, init.node)
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
Bool RigidBody::update(SceneNode& node, F32 prevTime, F32 crntTime,
	UpdateType updateType)
{
	if(updateType == ASYNC_UPDATE)
	{
		return false;
	}

	MoveComponent* move = node.tryGetComponent<MoveComponent>();
	if(ANKI_UNLIKELY(move == nullptr))
	{
		return false;
	}

	//Bool moveUpdated = move->bitsEnabled(MoveComponent::MF_MARKED_FOR_UPDATE);
	//Transform oldTrf = move->getLocalTransform();

	Bool mstateUpdated = motionState.getUpdated();

	// Sync from motion state to move component
	if(mstateUpdated)
	{
		// Set local transform and preserve scale
		Transform newTrf;
		F32 originalScale = move->getLocalTransform().getScale();
		newTrf = toAnki(motionState.getWorldTransform2());
		newTrf.setScale(originalScale);
		move->setLocalTransform(newTrf);

		// Set the flag
		motionState.setUpdated(false);
	}

	// Sync from move component to motion state
	/*if(moveUpdated)
	{
		setWorldTransform(toBt(oldTrf));
		activate();
	}*/

	return mstateUpdated;
}

} // end namespace anki
