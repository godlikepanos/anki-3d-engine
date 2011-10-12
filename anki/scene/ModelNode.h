#ifndef ANKI_SCENE_MODEL_NODE_H
#define ANKI_SCENE_MODEL_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/resource/RsrcPtr.h"
#include "anki/scene/ModelPatchNode.h"
#include "anki/collision/Obb.h"
#include <boost/array.hpp>
#include <vector>
#include <boost/range/iterator_range.hpp>


namespace anki {


class Model;


/// The model scene node
class ModelNode: public SceneNode
{
	public:
		typedef boost::iterator_range<std::vector<ModelPatchNode*>::
			const_iterator> ConstRangeModelPatchNodes;

		typedef boost::iterator_range<std::vector<ModelPatchNode*>::
			iterator> MutableRangeModelPatchNodes;

		ModelNode(Scene& scene, ulong flags, SceneNode* parent);
		virtual ~ModelNode();

		/// @name Accessors
		/// @{
		ConstRangeModelPatchNodes getModelPatchNodes() const
		{
			return ConstRangeModelPatchNodes(patches.begin(), patches.end());
		}
		MutableRangeModelPatchNodes getModelPatchNodes()
		{
			return MutableRangeModelPatchNodes(patches.begin(), patches.end());
		}

		const Model& getModel() const
		{
			return *model;
		}

		const CollisionShape*
			getVisibilityCollisionShapeWorldSpace() const
		{
			return &visibilityShapeWSpace;
		}
		/// @}

		/// Initialize the node
		/// - Load the resource
		void init(const char* filename);

		/// Update the bounding shape
		void moveUpdate();

	private:
		RsrcPtr<Model> model;
		std::vector<ModelPatchNode*> patches;
		Obb visibilityShapeWSpace;
};


} // end namespace


#endif
