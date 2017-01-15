// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/PhysicsObject.h>
#include <CustomPlayerControllerManager.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// The implementation of the Newton manager.
class CharacterControllerManager : public CustomPlayerControllerManager
{
public:
	PhysicsWorld* m_world;

	CharacterControllerManager(PhysicsWorld* world);

	~CharacterControllerManager()
	{
	}

	void ApplyPlayerMove(CustomPlayerController* const controller, dFloat timestep);
};

/// Init info for PhysicsPlayerController.
class PhysicsPlayerControllerInitInfo
{
public:
	F32 m_mass = 83.0;
	F32 m_innerRadius = 0.30;
	F32 m_outerRadius = 0.50;
	F32 m_height = 1.9;
	F32 m_stepHeight = 1.9 * 0.33;
	Vec4 m_position = Vec4(0.0);
};

/// A player controller that walks the world.
class PhysicsPlayerController final : public PhysicsObject
{
	friend class CharacterControllerManager;

public:
	PhysicsPlayerController(PhysicsWorld* world)
		: PhysicsObject(PhysicsObjectType::PLAYER_CONTROLLER, world)
	{
	}

	~PhysicsPlayerController();

	ANKI_USE_RESULT Error create(const PhysicsPlayerControllerInitInfo& init);

	// Update the state machine
	void setVelocity(F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed, const Vec4& forwardDir)
	{
		m_forwardSpeed = forwardSpeed;
		m_strafeSpeed = strafeSpeed;
		m_jumpSpeed = jumpSpeed;
		m_forwardDir = forwardDir;
	}

	void moveToPosition(const Vec4& position);

	const Transform& getTransform(Bool& updated)
	{
		updated = m_updated;
		m_updated = false;
		return m_trf;
	}

private:
	CustomPlayerController* m_newtonPlayer = nullptr;
	Transform m_trf = Transform::getIdentity();
	Bool m_updated = true;

	// State
	F32 m_forwardSpeed = 0.0;
	F32 m_strafeSpeed = 0.0;
	F32 m_jumpSpeed = 0.0;
	Vec4 m_forwardDir = Vec4(0.0, 0.0, -1.0, 0.0);

	static void onTransformUpdateCallback(const NewtonBody* body, const dFloat* matrix, int threadIndex)
	{
		ANKI_ASSERT(body && matrix);
		Transform trf = Transform(toAnki(dMatrix(matrix)));
		void* ud = NewtonBodyGetUserData(body);
		ANKI_ASSERT(ud);
		static_cast<PhysicsPlayerController*>(ud)->onTransformUpdate(trf);
	}

	void onTransformUpdate(const Transform& trf)
	{
		if(trf != m_trf)
		{
			m_updated = true;
			m_trf = trf;
		}
	}
};
/// @}

} // end namespace anki
