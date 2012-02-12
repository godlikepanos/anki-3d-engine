#include "anki/scene/ModelNode.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>


namespace anki {


//==============================================================================
// ModelPatchNode                                                              =
//==============================================================================

//==============================================================================
ModelPatchNode::ModelPatchNode(const ModelPatch* modelPatch_,
	const char* name, Scene* scene,
	uint movableFlags, Movable* movParent)
	: SceneNode(name, scene), Movable(movableFlags, movParent, *this),
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
	: SceneNode(name, scene), Movable(movableFlags, movParent, *this),
		Spatial(&obb)
{
	model.load(modelFname);

	uint i = 0;
	BOOST_FOREACH(const ModelPatch& patch, model->getModelPatches())
	{
		std::string name = model.getResourceName()
			+ boost::lexical_cast<std::string>(i);

		ModelPatchNode* mpn = new ModelPatchNode(&patch, name.c_str(),
			scene, Movable::MF_IGNORE_LOCAL_TRANSFORM, this);

		patches.push_back(mpn);
		++i;
	}
}


//==============================================================================
ModelNode::~ModelNode()
{}


} // end namespace
