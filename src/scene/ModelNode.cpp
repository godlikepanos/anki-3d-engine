#include <boost/foreach.hpp>
#include "ModelNode.h"
#include "rsrc/Model.h"
#include "rsrc/Skeleton.h"


//==============================================================================
// Constructors & destructor                                                   =
//==============================================================================

ModelNode::ModelNode(Scene& scene, ulong flags, SceneNode* parent)
:	SceneNode(CID_MODEL_NODE, scene, flags, parent)
{}


ModelNode::ModelNode(ClassId cid, Scene& scene, ulong flags, SceneNode* parent)
:	SceneNode(cid, scene, flags, parent)
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
		patches.push_back(new ModelPatchNode(patch, this));
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
