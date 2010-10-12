#ifndef SKEL_ANIM_H
#define SKEL_ANIM_H

#include "Resource.h"
#include "Math.h"


/// Skeleton animation resource
///
/// The format will be changed to:
///
/// @code
/// skeletonAnimation
/// {
/// 	name same-as-file
/// 	keyframes {<integer> <integer> ... <integer>}
/// 	bones
/// 	{
/// 		num <integer>
/// 		boneAnims
/// 		{
/// 			boneAnim
/// 			{
/// 				hasAnim <true | false>
/// 				[bonePoses
/// 				{
/// 					bonePose
/// 					{
/// 						quat {<float> <float> <float> <float>}
/// 						trf {<float> <float> <float>}
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
/// @endcode
class SkelAnim: public Resource
{
	public:
		/// Bone pose
		class BonePose
		{
			public:
				Quat rotation;
				Vec3 translation;
		};

		/// Bone animation
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

		/// Implements Resource::loat
		void load(const char* filename);
};


inline SkelAnim::SkelAnim():
	Resource(RT_SKEL_ANIM)
{}

#endif
