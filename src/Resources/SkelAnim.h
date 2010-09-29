#ifndef SKELANIM_H
#define SKELANIM_H

#include "Common.h"
#include "Resource.h"
#include "Math.h"


/// Skeleton animation resource
///
/// The format will be changed to:
///
/// skeletonAnimation
/// {
/// 	name <same-as-file>
/// 	keyframes {<val> <val> ... <val>}
/// 	bones
/// 	{
/// 		num <val>
/// 		boneAnims
/// 		{
/// 			boneAnim
/// 			{
/// 				hasAnim <true | false>
/// 				[bonePoses
/// 				{
/// 					bonePose
/// 					{
/// 						quat {<val> <val> <val> <val>}
/// 						trf {<val> <val> <val>}
/// 					}
/// 					...
/// 					bonePose
/// 					{
/// 						...
/// 					}
/// 				}]
/// 			}
/// 			...
/// 			boneAnim
/// 			{
/// 				...
/// 			}
/// 		}
/// 	}
/// }
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
