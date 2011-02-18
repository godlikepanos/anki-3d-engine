#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include <boost/array.hpp>
#include "SceneNode.h"
#include "RsrcPtr.h"
#include "Properties.h"
#include "ModelPatchNode.h"
#include "Vec.h"
#include "Sphere.h"


class Model;


/// The model scene node
class ModelNode: public SceneNode
{
	public:
		ModelNode(): SceneNode(SNT_MODEL, true, NULL) {}

		/// @name Accessors
		/// @{
		const Model& getModel() const {return *model;}
		const Vec<ModelPatchNode*>& getModelPatchNodes() const {return patches;}
		const Sphere& getBoundingShapeWSpace() const {return boundingShapeWSpace;}
		/// @}

		/// Initialize the node
		/// - Load the resource
		void init(const char* filename);

		/// Update the bounding shape
		void moveUpdate();

		void frameUpdate() {}

		/// @name Accessors
		/// @{
		const Vec<ModelPatchNode*>& getModelPatchNodees() const {return patches;}
		/// @}

	private:
		RsrcPtr<Model> model;
		Vec<ModelPatchNode*> patches;
		Sphere boundingShapeWSpace;
};


#endif
