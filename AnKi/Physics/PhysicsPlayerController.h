// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics/PhysicsObject.h>
#include <AnKi/Util/ClassWrapper.h>

namespace anki {

/// @addtogroup physics
/// @{

/// Init info for PhysicsPlayerController.
class PhysicsPlayerControllerInitInfo
{
public:
	F32 m_mass = 83.0f;
	F32 m_innerRadius = 0.30f;
	F32 m_outerRadius = 0.50f;
	F32 m_height = 1.9f;
	F32 m_stepHeight = 1.9f * 0.33f;
	Vec3 m_position = Vec3(0.0f);
};

/// A player controller that walks the world.
class PhysicsPlayerController final : public PhysicsFilteredObject
{
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kPlayerController)

public:
	// Update the state machine
	void setVelocity(F32 forwardSpeed, [[maybe_unused]] F32 strafeSpeed, [[maybe_unused]] F32 jumpSpeed,
					 const Vec4& forwardDir)
	{
		m_controller->setWalkDirection(toBt((forwardDir * forwardSpeed).xyz()));
	}

	/// This is a deferred operation, will happen on the next PhysicsWorld::update.
	void moveToPosition(const Vec3& position)
	{
		m_moveToPosition = position;
	}

	Transform getTransform()
	{
		return toAnki(m_ghostObject->getWorldTransform());
	}

private:
	ClassWrapper<btPairCachingGhostObject> m_ghostObject;
	ClassWrapper<btCapsuleShape> m_convexShape;
	ClassWrapper<btKinematicCharacterController> m_controller;
	Vec3 m_moveToPosition = Vec3(kMaxF32);

	PhysicsPlayerController(PhysicsWorld* world, const PhysicsPlayerControllerInitInfo& init);

	~PhysicsPlayerController();

	void registerToWorld() override;

	void unregisterFromWorld() override;

	/// Called in PhysicsWorld::update.
	void moveToPositionForReal();
};
/// @}

} // end namespace anki
