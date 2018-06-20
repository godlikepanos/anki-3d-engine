// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsPlayerController.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

PhysicsPlayerController::PhysicsPlayerController(PhysicsWorld* world, const PhysicsPlayerControllerInitInfo& init)
	: PhysicsObject(PhysicsObjectType::PLAYER_CONTROLLER, world)
{
	btTransform trf = toBt(Transform(init.m_position.xyz0(), Mat3x4::getIdentity(), 1.0f));

	m_convexShape = getAllocator().newInstance<btCapsuleShapeZ>(init.m_outerRadius, init.m_height);

	m_ghostObject = getAllocator().newInstance<btPairCachingGhostObject>();
	m_ghostObject->setWorldTransform(trf);
	m_ghostObject->setCollisionShape(m_convexShape);
	m_ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

	m_controller = getAllocator().newInstance<btKinematicCharacterController>(
		m_ghostObject, m_convexShape, init.m_stepHeight, btVector3(0, 1, 0));

	getWorld().getBtWorld()->addCollisionObject(m_ghostObject,
		btBroadphaseProxy::CharacterFilter,
		btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	getWorld().getBtWorld()->addAction(m_controller);
}

PhysicsPlayerController::~PhysicsPlayerController()
{
	getWorld().getBtWorld()->removeAction(m_controller);
	getWorld().getBtWorld()->removeCollisionObject(m_ghostObject);

	getAllocator().deleteInstance(m_controller);
	getAllocator().deleteInstance(m_ghostObject);
	getAllocator().deleteInstance(m_convexShape);
}

void PhysicsPlayerController::moveToPosition(const Vec4& position)
{
	m_ghostObject->setWorldTransform(toBt(Transform(position, Mat3x4::getIdentity(), 1.0f)));
}

} // end namespace anki
