#include "ModelNode.h"
#include "Model.h"
#include "Skeleton.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void ModelNode::init(const char* filename)
{
	model.loadRsrc(filename);

	for(uint i = 0; i < model->getModelPatches().size(); i++)
	{
		patches.push_back(new ModelNodePatch(*this, model->getModelPatches()[i]));
	}
}
