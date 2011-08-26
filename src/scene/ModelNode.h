#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include <boost/array.hpp>
#include "SceneNode.h"
#include "rsrc/RsrcPtr.h"
#include "util/Accessors.h"
#include "ModelPatchNode.h"
#include "util/Vec.h"
#include "cln/Obb.h"


class Model;


/// The model scene node
class ModelNode: public SceneNode
{
	public:
		ModelNode(bool inheritParentTrfFlag, SceneNode* parent);
		virtual ~ModelNode();

		/// @name Accessors
		/// @{
		GETTER_RW(Vec<ModelPatchNode*>, patches, getModelPatchNodes)
		const Model& getModel() const {return *model;}
		GETTER_R(cln::Obb, visibilityShapeWSpace, getVisibilityShapeWSpace)
		/// @}

		/// Initialize the node
		/// - Load the resource
		void init(const char* filename);

		/// Update the bounding shape
		void moveUpdate();

	private:
		RsrcPtr<Model> model;
		Vec<ModelPatchNode*> patches;
		cln::Obb visibilityShapeWSpace;
};


#endif
