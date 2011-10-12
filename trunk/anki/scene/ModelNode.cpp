#include <boost/foreach.hpp>
#include "anki/scene/ModelNode.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"


namespace anki {


//==============================================================================
// Constructors & destructor                                                   =
//==============================================================================

ModelNode::ModelNode(Scene& scene, ulong flags, SceneNode* parent)
:	SceneNode(SNT_MODEL_NODE, scene, flags, parent)
{}


ModelNode::~ModelNode()
{}


//==============================================================================
// init                                                                        =
//==============================================================================
void ModelNode::init(const char* filename)
{
	model.loadRsrc(filename);

	BOOST_FOREACH(const ModelPatch& patch, model->getModelPatches())
	{
		patches.push_back(new ModelPatchNode(patch, *this));
	}
}


//==============================================================================
// moveUpdate                                                                  =
//==============================================================================
void ModelNode::moveUpdate()
{
	// Update bounding shape
	visibilityShapeWSpace = model->getVisibilityShape().getTransformed(
		getWorldTransform());
}


} // end namespace
