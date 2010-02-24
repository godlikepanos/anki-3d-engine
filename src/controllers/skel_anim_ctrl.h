#ifndef _SKEL_ANIM_CTRL_H_
#define _SKEL_ANIM_CTRL_H_

#include "common.h"
#include "controller.h"
#include "gmath.h"

class Skeleton;
class SkelAnim;
class skel_node_t;


/// Skeleton animation controller
class skel_anim_ctrl_t: public controller_t
{
	private:
		void Interpolate( SkelAnim* animation, float frame );
		void UpdateBoneTransforms();
		void Deform();

	public:
		SkelAnim*  skel_anim; ///< Skeleton animation resource
		skel_node_t*  skel_node;
		Vec<vec3_t> heads;
		Vec<vec3_t> tails;
		Vec<mat3_t> bone_rotations;
		Vec<vec3_t> Boneranslations;
		float step;
		float frame;

		skel_anim_ctrl_t( skel_node_t* skel_node_ );
		void Update( float time );
};


#endif
