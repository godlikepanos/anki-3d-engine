#include "anki/scene/ModelNode.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"

namespace anki {

//==============================================================================
// ModelPatchNode                                                              =
//==============================================================================

//==============================================================================
ModelPatchNode::ModelPatchNode(const ModelPatchBase *modelPatch_,
	const char* name, SceneGraph* scene,
	U32 movableFlags, Movable* movParent)
	:	SceneNode(name, scene),
		Movable(movableFlags, movParent, *this, getSceneAllocator()),
		Renderable(getSceneAllocator()),
		Spatial(&obb), modelPatch(modelPatch_)
{
	sceneNodeProtected.movable = this;
	sceneNodeProtected.renderable = this;
	sceneNodeProtected.spatial = this;

	Renderable::init(*this);
}

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
ModelNode::ModelNode(const char* modelFname,
	const char* name, SceneGraph* scene,
	uint movableFlags, Movable* movParent)
	: 	SceneNode(name, scene),
		Movable(movableFlags, movParent, *this, getSceneAllocator()),
		patches(getSceneAllocator())
{
	sceneNodeProtected.movable = this;

	model.load(modelFname);

	patches.reserve(model->getModelPatches().size());

	U i = 0;
	for(const ModelPatchBase* patch : model->getModelPatches())
	{
		std::string name_ = name + std::to_string(i);

		ModelPatchNode* mpn = ANKI_NEW(ModelPatchNode, getSceneAllocator(),
			patch, name_.c_str(),
			scene, Movable::MF_IGNORE_LOCAL_TRANSFORM, this);

		patches.push_back(mpn);
		++i;
	}
}

//==============================================================================
ModelNode::~ModelNode()
{
	for(ModelPatchNode* patch : patches)
	{
		ANKI_DELETE(patch, getSceneAllocator());
	}
}

} // end namespace anki
