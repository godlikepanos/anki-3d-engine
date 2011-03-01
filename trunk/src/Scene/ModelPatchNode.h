#ifndef MODEL_PATCH_NODE_H
#define MODEL_PATCH_NODE_H

#include "PatchNode.h"
#include "Obb.h"


class ModelNode;


/// A fragment of the ModelNode
class ModelPatchNode: public PatchNode
{
	public:
		ModelPatchNode(const ModelPatch& modelPatch, ModelNode* parent);

		const Obb& getVisibilityShapeWSpace() const {return visibilityShapeWSpace;}

		virtual void moveUpdate(); ///< Update the visibility shape
		virtual void frameUpdate() {}

	private:
		const ModelPatch& modelPatch;
		Obb visibilityShapeWSpace;
};


#endif
