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
	m_ghostObject = getAllocator().newInstance<btPairCachingGhostObject>();
	m_convexShape = getAllocator().newInstance<btCapsuleShape>(init.m_outerRadius, init.m_height);

	m_controller = getAllocator().newInstance<btKinematicCharacterController>(
		m_ghostObject, m_convexShape, init.m_stepHeight, btVector3(0, 1, 0));
}

PhysicsPlayerController::~PhysicsPlayerController()
{
	// TODO
}

void PhysicsPlayerController::moveToPosition(const Vec4& position)
{
	ANKI_ASSERT(!"TODO");
}

} // end namespace anki
