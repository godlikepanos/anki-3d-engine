// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Math.h>
#include <anki/util/Vector.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// Bone pose
struct BonePose
{
public:
	BonePose(const Quat& r, const Vec3& trs);

	/// Copy constructor
	BonePose(const BonePose& b);

	/// @name Accessors
	/// @{
	const Quat& getRotation() const
	{
		return rotation;
	}

	const Vec3& getTranslation() const
	{
		return translation;
	}
	/// @}

private:
	Quat rotation;
	Vec3 translation;
};

inline BonePose::BonePose(const Quat& r, const Vec3& trs)
	: rotation(r)
	, translation(trs)
{
}

inline BonePose::BonePose(const BonePose& b)
	: rotation(b.rotation)
	, translation(b.translation)
{
}

/// Bone animation
class BoneAnim
{
	friend class SkelAnim;

public:
	/// @name Accessors
	/// @{
	const Vector<BonePose>& getBonePoses() const
	{
		return bonePoses;
	}
	/// @}

private:
	/// The poses for every keyframe. Its empty if the bone doesnt have any animation
	Vector<BonePose> bonePoses;
};

/// Skeleton animation resource
///
/// The binary file format:
/// @code
/// magic: ANKIANIM
/// uint: Keyframes number m
/// m * uint: The keyframe numbers
/// uint: Bone animations num, n, The n is equal to skeleton's bone number
/// n * BoneAnim: Bone animations
///
/// BoneAnim:
/// uint: Bone poses number. Its zero or m
/// m * BonePose: Bone poses
///
/// BonePose:
/// 4 * float: Quaternion for rotation
/// 3 * float: Traslation
/// @endcode
class SkelAnim
{
public:
	/// @name Accessors
	/// @{
	const Vector<U32>& getKeyframes() const
	{
		return keyframes;
	}

	U32 getFramesNum() const
	{
		return framesNum;
	}

	const Vector<BoneAnim>& getBoneAnimations() const
	{
		return boneAnims;
	}
	/// @}

	/// Load
	void load(const char* filename);

private:
	Vector<U32> keyframes;
	U32 framesNum;
	Vector<BoneAnim> boneAnims;
};
/// @}

} // end namespace
