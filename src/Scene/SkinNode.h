#ifndef SKIN_NODE_H
#define SKIN_NODE_H

#include "SceneNode.h"
#include "SkinPatchNode.h"
#include "Vec.h"
#include "Math.h"


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
		const Vec<Vec3>& getHeads() const {return heads;}
		Vec<Vec3>& getHeads() {return heads;}

		const Vec<Vec3>& getTails() const {return tails;}
		Vec<Vec3>& getTails() {return tails;}

		const Vec<Mat3>& getBoneRotations() const {return boneRotations;}
		Vec<Mat3>& getBoneRotations() {return boneRotations;}

		const Vec<Vec3>& getBoneTranslations() const {return boneTranslations;}
		Vec<Vec3>& getBoneTranslations() {return boneTranslations;}

		const Skin& getSkin() const {return *skin;}

		const Sphere& getBoundingShapeWSpace() const {return boundingShapeWSpace;}
		/// @}

		void init(const char* filename);

		/// Update boundingShapeWSpace from bone tails (not bone and heads cause its faster that way). The tails come from
		/// the previous frame
		void updateTrf();

	private:
		RsrcPtr<Skin> skin; ///< The resource
		Vec<SkinPatchNode*> patches;
		Sphere boundingShapeWSpace;

		Vec<Vec3> heads;
		Vec<Vec3> tails;
		Vec<Mat3> boneRotations;
		Vec<Vec3> boneTranslations;
};


#endif
