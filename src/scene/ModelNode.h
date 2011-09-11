#ifndef MODEL_NODE_H
#define MODEL_NODE_H

#include <boost/array.hpp>
#include "SceneNode.h"
#include "rsrc/RsrcPtr.h"
#include "util/Accessors.h"
#include "ModelPatchNode.h"
#include "util/Vec.h"
#include "cln/Obb.h"


class Model;


/// The model scene node
class ModelNode: public SceneNode
{
	public:
		ModelNode(ClassId cid, Scene& scene, ulong flags, SceneNode* parent);
		ModelNode(Scene& scene, ulong flags, SceneNode* parent);
		virtual ~ModelNode();

		static bool classof(const SceneNode* x);

		/// @name Accessors
		/// @{
		GETTER_RW(Vec<ModelPatchNode*>, patches, getModelPatchNodes)
		const Model& getModel() const {return *model;}
		GETTER_R(Obb, visibilityShapeWSpace, getVisibilityShapeWSpace)
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


inline bool ModelNode::classof(const SceneNode* x)
{
	return x->getClassId() == CID_MODEL_NODE ||
		x->getClassId() == CID_PARTICLE;
}


#endif
