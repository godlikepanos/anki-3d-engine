#ifndef _SKEL_ANIM_H_
#define _SKEL_ANIM_H_

#include "common.h"
#include "Resource.h"
#include "Math.h"


/// Skeleton animation
class SkelAnim: public Resource
{
	public:
		/// BonePose
		class BonePose
		{
			public:
				Quat rotation;
				Vec3 translation;
		};


		/// BoneAnim
		class BoneAnim
		{
			public:
				Vec<BonePose> keyframes; ///< The poses for every keyframe. Its empty if the bone doesnt have any animation

				BoneAnim() {}
				~BoneAnim() { if(keyframes.size()) keyframes.clear();}
		};
		
		Vec<uint> keyframes;
		uint frames_num;

		Vec<BoneAnim> bones;

		SkelAnim() {}
		~SkelAnim() {}
		bool load( const char* filename );
		void unload() { keyframes.clear(); bones.clear(); frames_num=0; }
};



#endif
