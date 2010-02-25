#ifndef _SKEL_ANIM_CTRL_H_
#define _SKEL_ANIM_CTRL_H_

#include "Common.h"
#include "Controller.h"
#include "Math.h"

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
		Vec<Vec3> heads;
		Vec<Vec3> tails;
		Vec<Mat3> boneRotations;
		Vec<Vec3> boneTranslations;
		float step;
		float frame;

		SkelAnimCtrl( SkelNode* skel_node_ );
		void update( float time );
};


#endif
