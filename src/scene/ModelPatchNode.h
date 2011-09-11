#ifndef MODEL_PATCH_NODE_H
#define MODEL_PATCH_NODE_H

#include "PatchNode.h"
#include "cln/Obb.h"


class ModelNode;


/// A fragment of the ModelNode
class ModelPatchNode: public PatchNode
{
	public:
		ModelPatchNode(const ModelPatch& modelPatch, ModelNode& parent);

		static bool classof(const SceneNode* x);

		GETTER_R(Obb, visibilityShapeWSpace, getVisibilityShapeWSpace)

		virtual void moveUpdate(); ///< Update the visibility shape

	private:
		Obb visibilityShapeWSpace;
};


inline bool ModelPatchNode::classof(const SceneNode* x)
{
	return x->getClassId() == CID_MODEL_PATCH_NODE;
}


#endif
