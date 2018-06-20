// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/PhysicsObject.h>

namespace anki
{

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
	Vec4 m_position = Vec4(0.0f);
};

/// A player controller that walks the world.
class PhysicsPlayerController final : public PhysicsObject
{
public:
	PhysicsPlayerController(PhysicsWorld* world, const PhysicsPlayerControllerInitInfo& init);

	~PhysicsPlayerController();

	// Update the state machine
	void setVelocity(F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed, const Vec4& forwardDir)
	{
		m_controller->setWalkDirection(toBt((forwardDir * forwardSpeed).xyz()));
	}

	void moveToPosition(const Vec4& position);

	Transform getTransform(Bool& updated)
	{
		Transform out = toAnki(m_ghostObject->getWorldTransform());
		updated = m_prevTrf != out;
		return out;
	}

private:
	btPairCachingGhostObject* m_ghostObject = nullptr;
	btCapsuleShape* m_convexShape = nullptr;
	btKinematicCharacterController* m_controller = nullptr;

	Transform m_prevTrf = Transform::getIdentity();
};
/// @}

} // end namespace anki
