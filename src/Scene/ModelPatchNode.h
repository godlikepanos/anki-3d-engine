#ifndef MODEL_PATCH_NODE_H
#define MODEL_PATCH_NODE_H

#include "PatchNode.h"
#include "Collision/Obb.h"


class ModelNode;


/// A fragment of the ModelNode
class ModelPatchNode: public PatchNode
{
	public:
		ModelPatchNode(const ModelPatch& modelPatch, ModelNode* parent);

		const Col::Obb& getVisibilityShapeWSpace() const {return visibilityShapeWSpace;}

		virtual void moveUpdate(); ///< Update the visibility shape
		virtual void frameUpdate(float /*prevUpdateTime*/, float /*crntTime*/) {}

	private:
		const ModelPatch& modelPatch;
		Col::Obb visibilityShapeWSpace;
};


#endif
