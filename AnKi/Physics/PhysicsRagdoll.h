// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics/Common.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// A bone of the ragdoll. One for every bone of the skeleton, even the ones without a capsule
class PhysicsRagdollBone
{
public:
	CString m_name; // Needs to live until the ragdoll is created

	Mat3x4 m_transform = Mat3x4::getIdentity(); // Rest pose relative to the parent bone. Same as Bone::getTransform()

	U32 m_parent = kMaxU32; // Index of the parent bone. It should be smaller than the index of this bone. kMaxU32 for the root

	// Everything is in bone space. In meters and without scale
	F32 m_capsuleRadius = 0.0f; // 0 means no capsule and no physics body. The part then follows its parent
	F32 m_capsuleHalfHeight = 0.0f; // Half the length of the cylinder part without the caps. Same as Jolt's CapsuleShape
	Vec3 m_capsulePosition = Vec3(0.0f); // The center of the capsule
	Euler m_capsuleRotation; // With zero rotation the capsule lies along +Y

	// Swing twist constraint to the closest ancestor that has a capsule. Ignored if there is none. The pivot is the part's head. Angles are in
	// radians and 0 locks the axis
	Euler m_constraintRotation; // In bone space. With zero rotation the twist axis is +Y and the plane axis is +X

	// Rotation of the parent's side on top of m_constraintRotation. Zero centers the cone on the rest pose. Used for one-sided joints (knees,
	// elbows). Jolt: frame2 = restModelRot * Mat3(m_constraintRotation), frame1 = frame2 * Mat3(m_constraintParentRotation)
	Euler m_constraintParentRotation;

	F32 m_twistMinAngle = 0.0f; // Around the twist axis
	F32 m_twistMaxAngle = 0.0f;
	F32 m_planeAngle = 0.0f; // Half cone angle. Same as Jolt's mPlaneHalfConeAngle
	F32 m_normalAngle = 0.0f; // Half cone angle. Same as Jolt's mNormalHalfConeAngle
};

class PhysicsRagdollInitInfo
{
public:
	ConstWeakArray<PhysicsRagdollBone> m_bones;

	F32 m_mass = 80.0f;
};

class PhysicsRagdoll : public PhysicsObjectBase
{
	ANKI_PHYSICS_COMMON_FRIENDS
	friend class PhysicsRagdollPtrDeleter;

public:
	// The world transform of every bone of the skeleton. Bones without a capsule follow their parent with their rest pose. Not thread-safe with
	// PhysicsWorld::update()
	void getBoneTransforms(WeakArray<Mat3x4> worldTrfs) const;

	// Teleports the ragdoll. One world transform per bone of the skeleton, without scale. Only the bones with a capsule are used. Not thread-safe
	// with PhysicsWorld::update()
	void setBoneTransforms(ConstWeakArray<Mat3x4> worldTrfs);

private:
	class Bone
	{
	public:
		Mat3x4 m_transform; // Rest pose relative to the parent
		U32 m_parent = kMaxU32;
		U32 m_part = kMaxU32; // kMaxU32 if the bone has no capsule
	};

	JPH::Ref<JPH::Ragdoll> m_ragdoll; // Not a ClassWrapper because Jolt allocates it in RagdollSettings::CreateRagdoll

	PhysicsDynamicArray<Bone> m_bones;

	PhysicsRagdoll();

	~PhysicsRagdoll();

	void init(const PhysicsRagdollInitInfo& init);
};

} // end namespace anki
