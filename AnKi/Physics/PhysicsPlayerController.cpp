// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics/PhysicsPlayerController.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>

namespace anki {

thread_local static StaticTempAllocator<1_MB> g_tempAllocator;

PhysicsPlayerController::PhysicsPlayerController()
	: PhysicsObjectBase(PhysicsObjectType::kPlayerController)
{
}

PhysicsPlayerController::~PhysicsPlayerController()
{
	m_jphCharacter.destroy();
}

void PhysicsPlayerController::init(const PhysicsPlayerControllerInitInfo& init)
{
	m_standingShape = PhysicsWorld::getSingleton().newCapsuleCollisionShape(init.m_standingHeight, init.m_waistWidth);
	m_crouchingShape = PhysicsWorld::getSingleton().newCapsuleCollisionShape(init.m_crouchingHeight, init.m_waistWidth);
	m_innerStandingShape =
		PhysicsWorld::getSingleton().newCapsuleCollisionShape(init.m_standingHeight * kInnerShapeFraction, init.m_waistWidth * kInnerShapeFraction);
	m_innerCrouchingShape =
		PhysicsWorld::getSingleton().newCapsuleCollisionShape(init.m_crouchingHeight * kInnerShapeFraction, init.m_waistWidth * kInnerShapeFraction);

	JPH::CharacterVirtualSettings settings;
	settings.SetEmbedded();

	settings.mMaxSlopeAngle = kMaxSlopeAngle;
	settings.mMaxStrength = kMaxStrength;
	settings.mShape = &m_standingShape->m_capsule;
	settings.mBackFaceMode = kBackFaceMode;
	settings.mCharacterPadding = kCharacterPadding;
	settings.mPenetrationRecoverySpeed = kPenetrationRecoverySpeed;
	settings.mPredictiveContactDistance = kPredictiveContactDistance;
	settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -init.m_waistWidth); // Accept contacts that touch the lower sphere of the capsule
	settings.mEnhancedInternalEdgeRemoval = kEnhancedInternalEdgeRemoval;
	settings.mInnerBodyShape = &m_innerStandingShape->m_capsule;
	settings.mInnerBodyLayer = JPH::ObjectLayer(PhysicsLayer::kPlayerController);

	m_jphCharacter.construct(&settings, toJPH(init.m_initialPosition), JPH::Quat::sIdentity(), ptrToNumber(static_cast<PhysicsObjectBase*>(this)),
							 &PhysicsWorld::getSingleton().m_jphPhysicsSystem);

	m_position = init.m_initialPosition;
	setUserData(init.m_userData);
}

void PhysicsPlayerController::prePhysicsUpdate(Second dt)
{
	const Bool bJump = m_input.m_jumpSpeed > 0.0f;
	const JPH::Vec3 inMovementDirection = toJPH(m_input.m_forwardDir);

	const Bool playerControlsHorizontalVelocity = m_controlMovementDuringJump || m_jphCharacter->IsSupported();
	if(playerControlsHorizontalVelocity)
	{
		// Smooth the player input
		m_desiredVelocity = kEnableCharacterInertia ? 0.25f * inMovementDirection * m_input.m_forwardSpeed + 0.75f * m_desiredVelocity
													: inMovementDirection * m_input.m_forwardSpeed;

		// True if the player intended to move
		m_allowSliding = !inMovementDirection.IsNearZero();
	}
	else
	{
		// While in air we allow sliding
		m_allowSliding = true;
	}

	// Update the character rotation and its up vector to match the up vector set by the user settings
	const JPH::Quat characterUpRotation = JPH::Quat::sEulerAngles(JPH::Vec3(0.0f, 0.0f, 0.0f));
	m_jphCharacter->SetUp(characterUpRotation.RotateAxisY());
	m_jphCharacter->SetRotation(characterUpRotation);

	// A cheaper way to update the character's ground velocity,
	// the platforms that the character is standing on may have changed velocity
	m_jphCharacter->UpdateGroundVelocity();

	// Determine new basic velocity
	const JPH::Vec3 currentVerticalVelocity = m_jphCharacter->GetLinearVelocity().Dot(m_jphCharacter->GetUp()) * m_jphCharacter->GetUp();
	const JPH::Vec3 groundVelocity = m_jphCharacter->GetGroundVelocity();
	JPH::Vec3 newVelocity;
	bool movingTowardsGround = (currentVerticalVelocity.GetY() - groundVelocity.GetY()) < 0.1f;
	if(m_jphCharacter->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround // If on ground
	   && (kEnableCharacterInertia
			   ? movingTowardsGround // Inertia enabled: And not moving away from ground
			   : !m_jphCharacter->IsSlopeTooSteep(m_jphCharacter->GetGroundNormal()))) // Inertia disabled: And not on a slope that is too steep
	{
		// Assume velocity of ground when on ground
		newVelocity = groundVelocity;

		// Jump
		if(bJump && movingTowardsGround)
		{
			newVelocity += m_input.m_jumpSpeed * m_jphCharacter->GetUp();
		}
	}
	else
	{
		newVelocity = currentVerticalVelocity;
	}

	// Gravity
	const auto& physicsSystem = *PhysicsWorld::getSingleton().m_jphPhysicsSystem;
	newVelocity += (characterUpRotation * physicsSystem.GetGravity()) * F32(dt);

	if(playerControlsHorizontalVelocity)
	{
		// Player input
		newVelocity += characterUpRotation * m_desiredVelocity;
	}
	else
	{
		// Preserve horizontal velocity
		const JPH::Vec3 currentHorizontalVelocity = m_jphCharacter->GetLinearVelocity() - currentVerticalVelocity;
		newVelocity += currentHorizontalVelocity;
	}

	// Update character velocity
	m_jphCharacter->SetLinearVelocity(newVelocity);

	// Stance switch
	if(m_crouching != m_input.m_crouching)
	{
		m_crouching = m_input.m_crouching;

		const Bool isStanding = m_jphCharacter->GetShape() == &m_standingShape->m_shapeBase;
		const JPH::Shape* shape = (isStanding) ? &m_crouchingShape->m_shapeBase : &m_standingShape->m_shapeBase;
		if(m_jphCharacter->SetShape(shape, 1.5f * physicsSystem.GetPhysicsSettings().mPenetrationSlop,
									physicsSystem.GetDefaultBroadPhaseLayerFilter(JPH::ObjectLayer(PhysicsLayer::kPlayerController)),
									physicsSystem.GetDefaultLayerFilter(JPH::ObjectLayer(PhysicsLayer::kPlayerController)), {}, {}, g_tempAllocator))
		{
			const JPH::Shape* innerShape = (isStanding) ? &m_innerCrouchingShape->m_capsule : &m_innerCrouchingShape->m_capsule;
			m_jphCharacter->SetInnerBodyShape(innerShape);
		}
	}

	if(m_input.m_updated)
	{
		m_input.m_updated = false;
		m_input = {};
	}

	// Settings for our update function
	JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
	if(!kEnableStickToFloor)
	{
		updateSettings.mStickToFloorStepDown = JPH::Vec3::sZero();
	}
	else
	{
		updateSettings.mStickToFloorStepDown = -m_jphCharacter->GetUp() * updateSettings.mStickToFloorStepDown.Length();
	}
	if(!kEnableWalkStairs)
	{
		updateSettings.mWalkStairsStepUp = JPH::Vec3::sZero();
	}
	else
	{
		updateSettings.mWalkStairsStepUp = m_jphCharacter->GetUp() * updateSettings.mWalkStairsStepUp.Length();
	}

	// Update the character position
	JPH::PhysicsSystem& system = *PhysicsWorld::getSingleton().m_jphPhysicsSystem;
	m_jphCharacter->ExtendedUpdate(F32(dt), -m_jphCharacter->GetUp() * system.GetGravity().Length(), updateSettings,
								   system.GetDefaultBroadPhaseLayerFilter(JPH::ObjectLayer(PhysicsLayer::kPlayerController)),
								   system.GetDefaultLayerFilter(JPH::ObjectLayer(PhysicsLayer::kPlayerController)), {}, {}, g_tempAllocator);
}

void PhysicsPlayerController::postPhysicsUpdate()
{
	const Vec3 newPos = toAnKi(m_jphCharacter->GetPosition());
	if(newPos != m_position)
	{
		m_position = newPos;
		++m_positionVersion;
	}
}

} // namespace anki
