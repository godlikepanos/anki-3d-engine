#ifndef MODEL_PATCH_NODE_H
#define MODEL_PATCH_NODE_H

#include "PatchNode.h"


class ModelNode;


/// A fragment of the ModelNode
class ModelPatchNode: public PatchNode
{
	public:
		ModelPatchNode(const ModelPatch& modelPatch, ModelNode* parent);
};


#endif
