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
	PROPERTY_RW(Vec<Vec3>, heads, setHeads, getHeads)
	PROPERTY_RW(Vec<Vec3>, tails, setTails, getTails)
	PROPERTY_RW(Vec<Mat3>, boneRotations, setBoneRotations, getBoneRotations)
	PROPERTY_RW(Vec<Vec3>, boneTranslations, setBoneTranslations, getBoneTranslations)

	public:
		SkelAnimModelNodeCtrl* skelAnimModelNodeCtrl; ///< @todo Clean this

		ModelNode(): SceneNode(SNT_MODEL) {}

		/// @name Accessors
		/// @{
		const Model& getModel() const {return *model;}
		/// @}

		/// @return True if the model support skeleton animation
		bool hasSkeleton() const {return boneRotations.size() > 0;}

		/// @todo
		void init(const char* filename);

	private:
		RsrcPtr<Model> model;
};


#endif
