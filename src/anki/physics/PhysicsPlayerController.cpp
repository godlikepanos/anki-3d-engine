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

	m_convexShape.init(init.m_outerRadius, init.m_height);

	m_ghostObject.init();
	m_ghostObject->setWorldTransform(trf);
	m_ghostObject->setCollisionShape(m_convexShape.get());
	m_ghostObject->setUserPointer(static_cast<PhysicsObject*>(this));
	setMaterialGroup(PhysicsMaterialBit::PLAYER);
	setMaterialMask(PhysicsMaterialBit::ALL);

	m_controller.init(m_ghostObject.get(), m_convexShape.get(), init.m_stepHeight, btVector3(0, 1, 0));

	{
		auto lock = getWorld().lockBtWorld();
		btDynamicsWorld* btworld = getWorld().getBtWorld();

		btworld->addCollisionObject(m_ghostObject.get(),
			btBroadphaseProxy::CharacterFilter,
			btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
		btworld->addAction(m_controller.get());
	}

	// Need to call this else the player is upside down
	moveToPosition(init.m_position);
}

PhysicsPlayerController::~PhysicsPlayerController()
{
	{
		auto lock = getWorld().lockBtWorld();
		getWorld().getBtWorld()->removeAction(m_controller.get());
		getWorld().getBtWorld()->removeCollisionObject(m_ghostObject.get());
	}

	m_controller.destroy();
	m_ghostObject.destroy();
	m_convexShape.destroy();
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
