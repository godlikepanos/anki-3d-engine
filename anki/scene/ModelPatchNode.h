#ifndef MODEL_PATCH_NODE_H
#define MODEL_PATCH_NODE_H

#include "anki/scene/PatchNode.h"
#include "anki/collision/Obb.h"


class ModelNode;


/// A fragment of the ModelNode
class ModelPatchNode: public PatchNode
{
	public:
		ModelPatchNode(const ModelPatch& modelPatch, ModelNode& parent);

		/// @copydoc SceneNode::getVisibilityCollisionShapeWorldSpace
		const CollisionShape*
			getVisibilityCollisionShapeWorldSpace() const
		{
			return &visibilityShapeWSpace;
		}


		void init(const char*)
		{}

		virtual void moveUpdate(); ///< Update the visibility shape

	private:
		Obb visibilityShapeWSpace;
};


#endif
