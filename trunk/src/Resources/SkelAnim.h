#ifndef SKEL_ANIM_H
#define SKEL_ANIM_H

#include "Math.h"
#include "Vec.h"
#include "Accessors.h"


/// Bone pose
struct BonePose
{
	public:
		BonePose(const Quat& r, const Vec3& trs): rotation(r), translation(trs) {}

		/// Copy contructor
		BonePose(const BonePose& b): rotation(b.rotation), translation(b.translation) {}

		/// @name Accessors
		/// @{
		GETTER_R(Quat, rotation, getRotation)
		GETTER_R(Vec3, translation, getTranslation)
		/// @}

	private:
		Quat rotation;
		Vec3 translation;
};


/// Bone animation
class BoneAnim
{
	friend class SkelAnim;

	public:
		/// @name Accessors
		/// @{
		GETTER_R(Vec<BonePose>, bonePoses, getBonePoses)
		/// @}

	private:
		Vec<BonePose> bonePoses; ///< The poses for every keyframe. Its empty if the bone doesnt have any animation
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
		GETTER_R(Vec<uint>, keyframes, getKeyframes)
		GETTER_R_BY_VAL(uint, framesNum, getFramesNum)
		GETTER_R(Vec<BoneAnim>, boneAnims, getBoneAnims)
		/// @}

		/// Load
		void load(const char* filename);

	private:
		Vec<uint> keyframes;
		uint framesNum;
		Vec<BoneAnim> boneAnims;
};


#endif
