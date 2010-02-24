#ifndef _SKEL_ANIM_CTRL_H_
#define _SKEL_ANIM_CTRL_H_

#include "common.h"
#include "Controller.h"
#include "gmath.h"

class Skeleton;
class SkelAnim;
class SkelNode;


/// Skeleton animation controller
class SkelAnimCtrl: public Controller
{
	private:
		void interpolate( SkelAnim* animation, float frame );
		void updateBoneTransforms();
		void deform();

	public:
		SkelAnim*  skelAnim; ///< Skeleton animation resource
		SkelNode*  skelNode;
		Vec<vec3_t> heads;
		Vec<vec3_t> tails;
		Vec<mat3_t> boneRotations;
		Vec<vec3_t> boneTranslations;
		float step;
		float frame;

		SkelAnimCtrl( SkelNode* skel_node_ );
		void update( float time );
};


#endif
