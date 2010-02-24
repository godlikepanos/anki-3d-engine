#ifndef _SKEL_ANIM_H_
#define _SKEL_ANIM_H_

#include "common.h"
#include "Resource.h"
#include "gmath.h"


/// Skeleton animation
class SkelAnim: public Resource
{
	public:
		/// BonePose
		class BonePose
		{
			public:
				quat_t rotation;
				vec3_t translation;
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
