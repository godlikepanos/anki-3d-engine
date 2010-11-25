#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include "SceneNode.h"
#include "RsrcPtr.h"
#include "Properties.h"


class Model;
class SkelAnimModelNodeCtrl;


/// The model scene node
class ModelNode: public SceneNode
{
	public:
		SkelAnimModelNodeCtrl* skelAnimModelNodeCtrl; ///< @todo Clean this

		ModelNode(): SceneNode(SNT_MODEL) {}

		/// @name Accessors
		/// @{
		const Model& getModel() const {return *model;}
		Vec<Vec3>& getHeads();
		Vec<Vec3>& getTails();
		Vec<Mat3>& getBoneRotations();
		Vec<Vec3>& getBoneTranslations();
		/// @}

		/// @return True if the model support skeleton animation
		bool hasSkeleton() const {return boneRotations.size() > 0;}

		/// @todo write this sucker
		void init(const char* filename) {}

	private:
		RsrcPtr<Model> model;
		Vec<Vec3> heads;
		Vec<Vec3> tails;
		Vec<Mat3> boneRotations;
		Vec<Vec3> boneTranslations;
};


inline Vec<Vec3>& ModelNode::getHeads()
{
	RASSERT_THROW_EXCEPTION(!hasSkeleton());
	return heads;
}


inline Vec<Vec3>& ModelNode::getTails()
{
	RASSERT_THROW_EXCEPTION(!hasSkeleton());
	return tails;
}


inline Vec<Mat3>& ModelNode::getBoneRotations()
{
	RASSERT_THROW_EXCEPTION(!hasSkeleton());
	return boneRotations;
}


inline Vec<Vec3>& ModelNode::getBoneTranslations()
{
	RASSERT_THROW_EXCEPTION(!hasSkeleton());
	return boneTranslations;
}


#endif
