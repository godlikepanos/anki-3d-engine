#ifndef _SKEL_ANIM_CTRL_H_
#define _SKEL_ANIM_CTRL_H_

#include "Common.h"
#include "Vec.h"
#include "Controller.h"
#include "Math.h"

class Skeleton;
class SkelAnim;
class SkelNode;


/// Skeleton animation controller
class SkelAnimCtrl: public Controller
{
	private:
		// The 3 steps of skeletal animation
		void interpolate(SkelAnim* animation, float frame);
		void updateBoneTransforms();
		void deform();  ///< Now with HW skinning it deforms only the debug skeleton

	public:
		SkelAnim* skelAnim; ///< Skeleton animation resource
		SkelNode* skelNode;
		Vec<Vec3> heads;
		Vec<Vec3> tails;
		Vec<Mat3> boneRotations;
		Vec<Vec3> boneTranslations;
		float step;
		float frame;

		SkelAnimCtrl(SkelNode* skelNode_);
		void update(float time);
};


#endif
