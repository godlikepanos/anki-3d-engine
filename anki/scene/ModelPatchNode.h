#ifndef ANKI_SCENE_MODEL_PATCH_NODE_H
#define ANKI_SCENE_MODEL_PATCH_NODE_H

#include "anki/scene/PatchNode.h"
#include "anki/collision/Obb.h"


namespace anki {


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

		/// Implements RenderableNode::getVao
		const Vao& getVao(const PassLevelKey& k) const
		{
			return getModelPatchRsrc().getVao(k);
		}

		void init(const char*)
		{}

		virtual void moveUpdate(); ///< Update the visibility shape

	private:
		Obb visibilityShapeWSpace;
};


} // end namespace


#endif
