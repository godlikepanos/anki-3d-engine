// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Math.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

constexpr U32 kMaxChildrenPerBone = 8;

// Skeleton bone
class Bone
{
	friend class SkeletonResource; // For loading

public:
	Bone() = default;

	~Bone() = default;

	CString getName() const
	{
		return m_name;
	}

	const Mat3x4& getTransform() const
	{
		return m_transform;
	}

	const Mat3x4& getVertexTransform() const
	{
		return m_vertTrf;
	}

	U32 getIndex() const
	{
		return m_idx;
	}

	ConstWeakArray<Bone*> getChildren() const
	{
		return ConstWeakArray<Bone*>((m_childrenCount) ? &m_children[0] : nullptr, m_childrenCount);
	}

	const Bone* getParent() const
	{
		return m_parent;
	}

	F32 getCapsuleRadius() const
	{
		return m_capsuleRadius;
	}

	F32 getCapsuleHalfHeight() const
	{
		return m_capsuleHalfHeight;
	}

	Vec3 getCapsulePosition() const
	{
		return m_capsulePosition;
	}

	Euler getCapsuleRotation() const
	{
		return m_capsuleRotation;
	}

	Euler getConstraintRotation() const
	{
		return m_constraintRotation;
	}

	Euler getConstraintParentRotation() const
	{
		return m_constraintParentRotation;
	}

	F32 getTwistMinAngle() const
	{
		return m_twistMinAngle;
	}

	F32 getTwistMaxAngle() const
	{
		return m_twistMaxAngle;
	}

	F32 getPlaneAngle() const
	{
		return m_planeAngle;
	}

	F32 getNormalAngle() const
	{
		return m_normalAngle;
	}

private:
	ResourceString m_name; // The name of the bone

	Mat3x4 m_transform; // See the class notes.
	Mat3x4 m_vertTrf;

	U32 m_idx;

	Bone* m_parent = nullptr;
	Array<Bone*, kMaxChildrenPerBone> m_children = {};
	U8 m_childrenCount = 0;

	// The capsule is used by physics. Everything is in bone space
	F32 m_capsuleRadius = 0.0f; // 0 means no capsule
	F32 m_capsuleHalfHeight = 0.0f; // Half the length of the cylinder part without the caps. Same as Jolt's CapsuleShape
	Vec3 m_capsulePosition = Vec3(0.0f); // The center of the capsule
	Euler m_capsuleRotation; // In radians. With zero rotation the capsule lies along +Y

	// Swing twist constraint to the parent bone. Used by physics. The pivot is the bone's head. Angles are in radians and 0 locks the axis
	Euler m_constraintRotation; // In bone space. With zero rotation the twist axis is +Y and the plane axis is +X

	// Rotation of the parent's side on top of m_constraintRotation. Zero centers the cone on the rest pose. Used for one-sided joints (knees,
	// elbows). Jolt: frame2 = boneRot * Mat3(m_constraintRotation), frame1 = frame2 * Mat3(m_constraintParentRotation)
	Euler m_constraintParentRotation;

	F32 m_twistMinAngle = 0.0f; // Around the twist axis
	F32 m_twistMaxAngle = 0.0f;
	F32 m_planeAngle = 0.0f; // Half cone angle. Same as Jolt's mPlaneHalfConeAngle
	F32 m_normalAngle = 0.0f; // Half cone angle. Same as Jolt's mNormalHalfConeAngle
};

// It contains the bones with their position and hierarchy
//
// XML file format:
//
// <skeleton>
//     <bones>
//          <bone name="X" transform="12 floats" boneTransform="12 floats" [parent="bone_name"] [capsuleRadius="float"]
//          [capsuleHalfHeight="float"] [capsulePosition="3 floats"] [capsuleRotation="3 floats Euler in degrees"]
//          [constraintRotation="3 floats Euler in degrees"] [constraintParentRotation="3 floats Euler in degrees"] [twistMinAngle="degrees"]
//          [twistMaxAngle="degrees"] [planeAngle="degrees"] [normalAngle="degrees"] />
// 		...
//     </bones>
// </skeleton>
class SkeletonResource : public ResourceObject
{
public:
	SkeletonResource(CString fname, U32 uuid)
		: ResourceObject(fname, uuid, ResourceType::kSkeletonResource)
	{
	}

	~SkeletonResource() = default;

	// Load file
	Error load(const ResourceFilename& filename, Bool async);

	ConstWeakArray<Bone> getBones() const
	{
		return m_bones;
	}

	const Bone* tryFindBone(CString name) const
	{
		for(const Bone& b : m_bones)
		{
			if(b.m_name == name)
			{
				return &b;
			}
		}

		return nullptr;
	}

	const Bone& getRootBone() const
	{
		return m_bones[m_rootBoneIdx];
	}

	U32 getRootBoneIndex() const
	{
		return m_rootBoneIdx;
	}

private:
	ResourceDynamicArray<Bone> m_bones;
	U32 m_rootBoneIdx = kMaxU32;
};

} // end namespace anki
