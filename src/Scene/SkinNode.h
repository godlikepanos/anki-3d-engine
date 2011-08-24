#ifndef SKIN_NODE_H
#define SKIN_NODE_H

#include "SceneNode.h"
#include "SkinPatchNode.h"
#include "Util/Vec.h"
#include "Math/Math.h"
#include "Util/Accessors.h"


class Skin;
class SkelAnimModelNodeCtrl;


/// A skin scene node
class SkinNode: public SceneNode
{
	public:
		SkelAnimModelNodeCtrl* skelAnimModelNodeCtrl; ///< @todo fix this crap

		SkinNode(bool inheritParentTrfFlag, SceneNode* parent);
		~SkinNode();

		/// @name Accessors
		/// @{
		GETTER_RW(Vec<Vec3>, heads, getHeads)
		GETTER_RW(Vec<Vec3>, tails, getTails)
		GETTER_RW(Vec<Mat3>, boneRotations, getBoneRotations)
		GETTER_RW(Vec<Vec3>, boneTranslations, getBoneTranslations)
		GETTER_R(Skin, *skin, getSkin)
		GETTER_R(Col::Obb, visibilityShapeWSpace, getVisibilityShapeWSpace)
		GETTER_R(Vec<SkinPatchNode*>, patches, getPatchNodes)

		GETTER_SETTER_BY_VAL(float, step, getStep, setStep)
		GETTER_SETTER_BY_VAL(float, frame, getFrame, setFrame)
		void setAnimation(const SkelAnim& anim_) {anim = &anim_;}
		const SkelAnim* getAnimation() const {return anim;}
		/// @}

		void init(const char* filename);

		/// Update boundingShapeWSpace from bone tails (not heads as well
		/// cause its faster that way). The tails come from the previous frame
		void moveUpdate();

		/// Update the animation stuff
		void frameUpdate(float prevUpdateTime, float crntTime);

	private:
		RsrcPtr<Skin> skin; ///< The resource
		Vec<SkinPatchNode*> patches;
		Col::Obb visibilityShapeWSpace;

		/// @name Animation stuff
		/// @{
		float step;
		float frame;
		const SkelAnim* anim; ///< The active skeleton animation
		/// @}

		/// @name Bone data
		/// @{
		Vec<Vec3> heads;
		Vec<Vec3> tails;
		Vec<Mat3> boneRotations;
		Vec<Vec3> boneTranslations;
		/// @}

		/// Interpolate
		/// @param[in] animation Animation
		/// @param[in] frame Frame
		/// @param[out] translations Translations vector
		/// @param[out] rotations Rotations vector
		static void interpolate(const SkelAnim& animation, float frame,
			Vec<Vec3>& translations, Vec<Mat3>& rotations);

		/// Calculate the global pose
		static void updateBoneTransforms(const Skeleton& skel,
			Vec<Vec3>& translations, Vec<Mat3>& rotations);

		/// Deform the heads and tails
		static void deformHeadsTails(const Skeleton& skeleton,
		    const Vec<Vec3>& boneTranslations, const Vec<Mat3>& boneRotations,
		    Vec<Vec3>& heads, Vec<Vec3>& tails);
};


#endif
