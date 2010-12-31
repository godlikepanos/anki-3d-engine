#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include <boost/array.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "SceneNode.h"
#include "RsrcPtr.h"
#include "Properties.h"
#include "ModelNodePatch.h"


class Model;


/// The model scene node
class ModelNode: public SceneNode
{
	public:
		ModelNode(): SceneNode(SNT_MODEL) {}

		/// @name Accessors
		/// @{
		const Model& getModel() const {return *model;}
		/// @}

		/// Initialize the node
		/// - Load the resource
		void init(const char* filename);

		/// @name Accessors
		/// @{
		const boost::ptr_vector<ModelNodePatch>& getModelNodePatches() const {return patches;}
		/// @}

	private:
		RsrcPtr<Model> model;
		boost::ptr_vector<ModelNodePatch> patches;
};


#endif
