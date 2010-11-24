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
		SkelAnimModelNodeCtrl* skelAnimModelNodeCtrl;

		ModelNode(): SceneNode(SNT_MODEL) {}

		/// @name Accessors
		/// @{
		const Model& getModel() const {return *model;}
		Vec<Vec3>& getHeads() {return heads;}
		Vec<Vec3>& getTails() {return tails;}
		Vec<Mat3>& getBoneRotations() {return boneRotations;}
		Vec<Vec3>& getBoneTranslations() {return boneTranslations;}
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


#endif
