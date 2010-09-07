#ifndef SKELANIM_H
#define SKELANIM_H

#include "Common.h"
#include "Resource.h"
#include "Math.h"


/// Skeleton animation resource
class SkelAnim: public Resource
{
	public:
		/**
		 * Bone pose
		 */
		class BonePose
		{
			public:
				Quat rotation;
				Vec3 translation;
		};

		/**
		 * Bone animation
		 */
		class BoneAnim
		{
			public:
				Vec<BonePose> keyframes; ///< The poses for every keyframe. Its empty if the bone doesnt have any animation

				BoneAnim() {}
				~BoneAnim() {}
		};
		
		Vec<uint> keyframes;
		uint framesNum;
		Vec<BoneAnim> bones;

		SkelAnim();
		~SkelAnim() {}
		bool load(const char* filename);
};


inline SkelAnim::SkelAnim():
	Resource(RT_SKEL_ANIM)
{}

#endif
