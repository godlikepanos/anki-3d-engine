#include "anki/scene/ModelNode.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"

namespace anki {

//==============================================================================
// ModelPatchNode                                                              =
//==============================================================================

//==============================================================================
ModelPatchNode::ModelPatchNode(const ModelPatch* modelPatch_,
	const char* name, Scene* scene,
	U32 movableFlags, Movable* movParent)
	: SceneNode(name, scene),
		Movable(movableFlags, movParent, *this, getSceneAllocator()),
		Renderable(getSceneAllocator()),
		Spatial(&obb), modelPatch(modelPatch_)
{
	Renderable::init(*this);
}

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
ModelNode::ModelNode(const char* modelFname,
	const char* name, Scene* scene,
	uint movableFlags, Movable* movParent)
	: 	SceneNode(name, scene),
		Movable(movableFlags, movParent, *this, getSceneAllocator())
{
	model.load(modelFname);

	uint i = 0;
	for(const ModelPatch* patch : model->getModelPatches())
	{
		std::string name_ = name + std::to_string(i);

		ModelPatchNode* mpn = new ModelPatchNode(patch, name_.c_str(),
			scene, Movable::MF_IGNORE_LOCAL_TRANSFORM, this);

		patches.push_back(mpn);
		++i;
	}
}

//==============================================================================
ModelNode::~ModelNode()
{}

} // end namespace anki
