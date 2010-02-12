#ifndef _SKEL_ANIM_CTRL_H_
#define _SKEL_ANIM_CTRL_H_

#include "common.h"
#include "controller.h"
#include "gmath.h"

class skeleton_t;
class skel_anim_t;
class skel_node_t;


/// Skeleton animation controller
class skel_anim_ctrl_t: public controller_t
{
	private:
		void Interpolate( skel_anim_t* animation, float frame );
		void UpdateBoneTransforms();
		void Deform();

	public:
		skel_anim_t*  skel_anim; ///< Skeleton animation resource
		skel_node_t*  skel_node;
		vec_t<vec3_t> heads;
		vec_t<vec3_t> tails;
		vec_t<mat3_t> bone_rotations;
		vec_t<vec3_t> bone_translations;
		float step;
		float frame;

		skel_anim_ctrl_t( skel_node_t* skel_node_ );
		void Update( float time );
};


#endif
