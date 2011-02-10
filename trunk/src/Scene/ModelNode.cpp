#include <boost/foreach.hpp>
#include "ModelNode.h"
#include "Model.h"
#include "Skeleton.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void ModelNode::init(const char* filename)
{
	model.loadRsrc(filename);

	BOOST_FOREACH(const ModelPatch& patch, model->getModelPatches())
	{
		patches.push_back(new ModelPatchNode(patch, this));
	}
}


//======================================================================================================================
// updateTrf                                                                                                           =
//======================================================================================================================
void ModelNode::updateTrf()
{
	// Update bounding shape
	boundingShapeWSpace = model->getBoundingShape().getTransformed(worldTransform);
}
