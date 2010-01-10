#ifndef _SKEL_ANIM_H_
#define _SKEL_ANIM_H_

#include "common.h"
#include "resource.h"
#include "gmath.h"


/// Skeleton animation
class skel_anim_t: public resource_t
{
	public:
		/// bone_pose_t
		class bone_pose_t
		{
			public:
				quat_t rotation;
				vec3_t translation;
		};


		/// bone_anim_t
		class bone_anim_t
		{
			public:
				vec_t<bone_pose_t> keyframes; ///< The poses for every keyframe. Its empty if the bone doesnt have any animation

				bone_anim_t() {}
				~bone_anim_t() { if(keyframes.size()) keyframes.clear();}
		};
		
		vec_t<uint> keyframes;
		uint frames_num;

		vec_t<bone_anim_t> bones;

		skel_anim_t() {}
		~skel_anim_t() {}
		bool Load( const char* filename );
		void Unload() { keyframes.clear(); bones.clear(); frames_num=0; }
};



#endif
