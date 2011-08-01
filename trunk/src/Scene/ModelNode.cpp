#include <boost/foreach.hpp>
#include "ModelNode.h"
#include "Resources/Model.h"
#include "Resources/Skeleton.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
ModelNode::ModelNode(bool inheritParentTrfFlag, SceneNode* parent)
:	SceneNode(SNT_MODEL, inheritParentTrfFlag, parent)
{}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
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
