#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include <boost/array.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "SceneNode.h"
#include "RsrcPtr.h"
#include "Properties.h"


class Model;
class SkelAnimModelNodeCtrl;
class ModelNodePatch;


/// The model scene node
class ModelNode: public SceneNode
{
	PROPERTY_RW(Vec<Vec3>, heads, getHeads, setHeads)
	PROPERTY_RW(Vec<Vec3>, tails, getTails, setTails)
	PROPERTY_RW(Vec<Mat3>, boneRotations, getBoneRotations, setBoneRotations)
	PROPERTY_RW(Vec<Vec3>, boneTranslations, getBoneTranslations, setBoneTranslations)

	public:
		SkelAnimModelNodeCtrl* skelAnimModelNodeCtrl; ///< @todo Clean this

		ModelNode(): SceneNode(SNT_MODEL) {}

		/// @name Accessors
		/// @{
		const Model& getModel() const {return *model;}
		/// @}

		/// @return True if the model support skeleton animation
		bool hasSkeleton() const {return boneRotations.size() > 0;}

		/// Initialize the node
		/// - Load the resource
		/// - Create the bone transformations
		/// - Create the patches
		void init(const char* filename);

	private:
		RsrcPtr<Model> model;
		boost::ptr_vector<ModelNodePatch> patches;
};


#endif
