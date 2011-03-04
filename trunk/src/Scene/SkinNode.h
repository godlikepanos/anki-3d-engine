#ifndef SKIN_NODE_H
#define SKIN_NODE_H

#include "SceneNode.h"
#include "SkinPatchNode.h"
#include "Vec.h"
#include "Math.h"
#include "Properties.h"


class Skin;
class SkelAnimModelNodeCtrl;


/// A skin scene node
class SkinNode: public SceneNode
{
	public:
		SkelAnimModelNodeCtrl* skelAnimModelNodeCtrl; ///< @todo fix this crap

		SkinNode(): SceneNode(SNT_SKIN, true, NULL) {}

		/// @name Accessors
		/// @{
		GETTER_RW(Vec<Vec3>, heads, getHeads)
		GETTER_RW(Vec<Vec3>, tails, getTails)
		GETTER_RW(Vec<Mat3>, boneRotations, getBoneRotations)
		GETTER_RW(Vec<Vec3>, boneTranslations, getBoneTranslations)
		const Skin& getSkin() const {return *skin;}
		GETTER_R(Obb, visibilityShapeWSpace, getVisibilityShapeWSpace)
		/// @}

		void init(const char* filename);

		/// Update boundingShapeWSpace from bone tails (not heads as well cause its faster that way). The tails come
		/// from the previous frame
		void moveUpdate();

	private:
		RsrcPtr<Skin> skin; ///< The resource
		Vec<SkinPatchNode*> patches;
		Obb visibilityShapeWSpace;

		/// @name Bone data
		/// @{
		Vec<Vec3> heads;
		Vec<Vec3> tails;
		Vec<Mat3> boneRotations;
		Vec<Vec3> boneTranslations;
		/// @}
};


#endif
