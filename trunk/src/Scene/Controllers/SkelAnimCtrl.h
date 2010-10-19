#ifndef SKEL_ANIM_CTRL_H
#define SKEL_ANIM_CTRL_H

#include "Vec.h"
#include "Controller.h"
#include "Math.h"
#include "RsrcPtr.h"

class Skeleton;
class SkelAnim;
class SkelNode;


/// Skeleton animation controller
class SkelAnimCtrl: public Controller
{
	public:
		RsrcPtr<SkelAnim> skelAnim; ///< Skeleton animation resource
		float step;
		float frame;

		SkelAnimCtrl(SkelNode& skelNode_);
		void update(float time);

	private:
		SkelNode& skelNode;

		/// @name The 3 steps of skeletal animation in 3 funcs
		/// @{
		void interpolate(const SkelAnim& animation, float frame);
		void updateBoneTransforms();
		void deform();  ///< Now with HW skinning it deforms only the debug skeleton
		/// @}
};


#endif
