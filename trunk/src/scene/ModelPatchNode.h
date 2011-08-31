#ifndef MODEL_PATCH_NODE_H
#define MODEL_PATCH_NODE_H

#include "PatchNode.h"
#include "cln/Obb.h"


class ModelNode;


/// A fragment of the ModelNode
class ModelPatchNode: public PatchNode
{
	public:
		ModelPatchNode(const ModelPatch& modelPatch, ModelNode* parent);

		GETTER_R(Obb, visibilityShapeWSpace, getVisibilityShapeWSpace)

		virtual void moveUpdate(); ///< Update the visibility shape

	private:
		Obb visibilityShapeWSpace;
};


#endif
