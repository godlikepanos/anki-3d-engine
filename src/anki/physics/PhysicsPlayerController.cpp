// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsPlayerController.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

PhysicsPlayerController::PhysicsPlayerController(PhysicsWorld* world, const PhysicsPlayerControllerInitInfo& init)
	: PhysicsFilteredObject(CLASS_TYPE, world)
{
	const btTransform trf = toBt(Transform(init.m_position.xyz0(), Mat3x4::getIdentity(), 1.0f));

	m_convexShape = getAllocator().newInstance<btCapsuleShape>(init.m_outerRadius, init.m_height);

	m_ghostObject = getAllocator().newInstance<btPairCachingGhostObject>();
	m_ghostObject->setWorldTransform(trf);
	m_ghostObject->setCollisionShape(m_convexShape);
	m_ghostObject->setUserPointer(static_cast<PhysicsObject*>(this));
	setMaterialGroup(PhysicsMaterialBit::PLAYER);
	setMaterialMask(PhysicsMaterialBit::ALL);

	m_controller = getAllocator().newInstance<btKinematicCharacterController>(
		m_ghostObject, m_convexShape, init.m_stepHeight, btVector3(0, 1, 0));

	{
		auto lock = getWorld().lockBtWorld();
		btDynamicsWorld* btworld = getWorld().getBtWorld();

		btworld->addCollisionObject(m_ghostObject,
			btBroadphaseProxy::CharacterFilter,
			btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
		btworld->addAction(m_controller);
	}

	// Need to call this else the player is upside down
	moveToPosition(init.m_position);
}

PhysicsPlayerController::~PhysicsPlayerController()
{
	{
		auto lock = getWorld().lockBtWorld();
		getWorld().getBtWorld()->removeAction(m_controller);
		getWorld().getBtWorld()->removeCollisionObject(m_ghostObject);
	}

	getAllocator().deleteInstance(m_controller);
	getAllocator().deleteInstance(m_ghostObject);
	getAllocator().deleteInstance(m_convexShape);
}

void PhysicsPlayerController::moveToPosition(const Vec4& position)
{
	auto lock = getWorld().lockBtWorld();

	getWorld().getBtWorld()->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(
		m_ghostObject->getBroadphaseHandle(), getWorld().getBtWorld()->getDispatcher());

	m_controller->reset(getWorld().getBtWorld());
	m_controller->warp(toBt(position.xyz()));
}

} // end namespace anki
