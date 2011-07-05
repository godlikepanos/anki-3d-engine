#ifndef SKEL_ANIM_MODEL_NODE_CTRL_H
#define SKEL_ANIM_MODEL_NODE_CTRL_H

#include "Util/Vec.h"
#include "Controller.h"
#include "Math/Math.h"
#include "Resources/RsrcPtr.h"
#include "Util/Accessors.h"


class Skeleton;
class SkelAnim;
class SkinNode;


/// SkelAnim controls a ModelNode
class SkelAnimModelNodeCtrl: public Controller
{
	public:
		SkelAnimModelNodeCtrl(SkinNode& skelNode_);

		/// @name Accessors
		/// @{
		float getStep() const {return step;}
		float& getStep() {return step;}
		void setStep(float s) {step = s;}
		/// @}

		void update(float time);
		void set(const SkelAnim* skelAnim_) {skelAnim = skelAnim_;}

	private:
		float step;
		float frame;
		const SkelAnim* skelAnim; ///< The active skeleton animation
		SkinNode& skinNode; ///< Know your father

		/// @name The 3 steps of skeletal animation in 3 methods
		/// @{

		/// Interpolate
		/// @param[in] animation Animation
		/// @param[in] frame Frame
		/// @param[out] translations Translations vector
		/// @param[out] rotations Rotations vector
		static void interpolate(const SkelAnim& animation, float frame,
			Vec<Vec3>& translations, Vec<Mat3>& rotations);

		static void updateBoneTransforms(const Skeleton& skel,
			Vec<Vec3>& translations, Vec<Mat3>& rotations);

		/// Now with HW skinning it deforms only the debug skeleton
		static void deform(const Skeleton& skel, const Vec<Vec3>& translations,
			const Vec<Mat3>& rotations,
			Vec<Vec3>& heads, Vec<Vec3>& tails);
		/// @}
};


#endif
