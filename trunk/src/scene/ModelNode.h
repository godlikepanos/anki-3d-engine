#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include <boost/array.hpp>
#include "SceneNode.h"
#include "rsrc/RsrcPtr.h"
#include "ModelPatchNode.h"
#include "util/Vec.h"
#include "cln/Obb.h"


class Model;


/// The model scene node
class ModelNode: public SceneNode
{
	public:
		ModelNode(Scene& scene, ulong flags, SceneNode* parent);
		virtual ~ModelNode();

		/// @name Accessors
		/// @{
		const Vec<ModelPatchNode*>& getModelPatchNodes() const {return patches;}
		Vec<ModelPatchNode*>& getModelPatchNodes() {return patches;}

		const Model& getModel() const {return *model;}

		const Obb& getVisibilityShapeWSpace() const
			{return visibilityShapeWSpace;}
		/// @}

		/// Initialize the node
		/// - Load the resource
		void init(const char* filename);

		/// Update the bounding shape
		void moveUpdate();

	private:
		RsrcPtr<Model> model;
		Vec<ModelPatchNode*> patches;
		Obb visibilityShapeWSpace;
};


#endif
