#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include <boost/array.hpp>
#include "SceneNode.h"
#include "Resources/RsrcPtr.h"
#include "Util/Accessors.h"
#include "ModelPatchNode.h"
#include "Util/Vec.h"
#include "Collision/Obb.h"


class Model;


/// The model scene node
class ModelNode: public SceneNode
{
	public:
		ModelNode(SceneNode* parent = NULL): SceneNode(SNT_MODEL, true, parent) {}
		virtual ~ModelNode();

		/// @name Accessors
		/// @{
		GETTER_RW(Vec<ModelPatchNode*>, patches, getModelPatchNodes)
		const Model& getModel() const {return *model;}
		const Col::Obb& getVisibilityShapeWSpace() const {return visibilityShapeWSpace;}
		/// @}

		/// Initialize the node
		/// - Load the resource
		void init(const char* filename);

		/// Update the bounding shape
		void moveUpdate();

		void frameUpdate(float /*prevUpdateTime*/, float /*crntTime*/) {}

	private:
		RsrcPtr<Model> model;
		Vec<ModelPatchNode*> patches;
		Col::Obb visibilityShapeWSpace;
};


#endif
