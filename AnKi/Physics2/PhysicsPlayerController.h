// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics2/Common.h>
#include <AnKi/Util/ClassWrapper.h>

namespace anki {
namespace v2 {

/// @addtogroup physics
/// @{

/// Init info for PhysicsPlayerController.
class PhysicsPlayerControllerInitInfo
{
public:
	F32 m_mass = 83.0f;

	F32 m_standingHeight = 1.9f;
	F32 m_crouchingHeight = 1.2f;
	F32 m_waistWidth = 0.3f;

	Vec3 m_initialPosition = Vec3(0.0f);

	Bool m_controlMovementDuringJump = true;

	void* m_userData = nullptr;
};

/// A player controller that walks the world.
class PhysicsPlayerController : public PhysicsObjectBase
{
	ANKI_PHYSICS_COMMON_FRIENDS
	friend class PhysicsPlayerControllerPtrDeleter;

public:
	// Update the state machine
	void updateState(F32 forwardSpeed, const Vec3& forwardDir, F32 jumpSpeed, Bool crouching)
	{
		m_input.m_forwardSpeed = forwardSpeed;
		m_input.m_forwardDir = forwardDir;
		m_input.m_jumpSpeed = jumpSpeed;
		m_input.m_crouching = crouching;
		m_input.m_updated = 1;
	}

	/// This is a deferred operation, will happen on the next PhysicsWorld::update.
	void moveToPosition(const Vec3& position)
	{
		m_jphCharacter->SetPosition(toJPH(position));
	}

	const Vec3& getPosition(U32* version = nullptr) const
	{
		if(version)
		{
			*version = m_positionVersion;
		}
		return m_position;
	}

private:
	static constexpr F32 kMaxSlopeAngle = toRad(45.0f);
	static constexpr F32 kMaxStrength = 100.0f;
	static constexpr JPH::EBackFaceMode kBackFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;
	static constexpr F32 kCharacterPadding = 0.02f;
	static constexpr F32 kPenetrationRecoverySpeed = 1.0f;
	static constexpr F32 kPredictiveContactDistance = 0.1f;
	static constexpr Bool kEnhancedInternalEdgeRemoval = false;
	static constexpr Bool kEnableCharacterInertia = true;
	static constexpr F32 kInnerShapeFraction = 0.9f;

	ClassWrapper<JPH::CharacterVirtual> m_jphCharacter;
	PhysicsCollisionShapePtr m_standingShape;
	PhysicsCollisionShapePtr m_crouchingShape;
	PhysicsCollisionShapePtr m_innerStandingShape; ///< Inner shape to recieve collision events.
	PhysicsCollisionShapePtr m_innerCrouchingShape;

	JPH::Vec3 m_desiredVelocity = JPH::Vec3::sZero();

	class
	{
	public:
		F32 m_forwardSpeed = 0.0f;
		Vec3 m_forwardDir;
		F32 m_jumpSpeed = 0.0f;
		U8 m_crouching : 1 = false;
		U8 m_updated : 1 = false;
	} m_input;

	Vec3 m_position;
	U32 m_positionVersion = 0;

	U32 m_arrayIndex : 29 = kMaxU32 >> 3u;
	U32 m_controlMovementDuringJump : 1 = true;
	U32 m_allowSliding : 1 = false;
	U32 m_crouching : 1 = false;

	PhysicsPlayerController()
		: PhysicsObjectBase(PhysicsObjectType::kPlayerController, nullptr)
	{
	}

	~PhysicsPlayerController();

	void init(const PhysicsPlayerControllerInitInfo& init);

	void prePhysicsUpdate(Second dt);

	void postPhysicsUpdate();
};
/// @}

} // namespace v2
} // end namespace anki
