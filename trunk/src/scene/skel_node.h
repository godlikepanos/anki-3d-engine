#ifndef _SKEL_NODE_H_
#define _SKEL_NODE_H_

#include "common.h"
#include "node.h"
#include "controller.h"
#include "gmath.h"


class skeleton_t;
class skel_anim_t;
class skel_node_t;


/// Skeleton animation controller
class skel_anim_controller_t: public controller_t<skel_node_t>
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

		skel_anim_controller_t( skel_node_t* skel_node_ ): skel_node( skel_node_ ) {}
		void Update();
};


/// Skeleton node
class skel_node_t: public node_t
{
	public:
		skeleton_t* skeleton;
		skel_anim_controller_t* skel_anim_controller;

		skel_node_t(): node_t( NT_SKELETON ) {}
		void Render();
		void Init( const char* filename );
		void Deinit();
};


#endif
