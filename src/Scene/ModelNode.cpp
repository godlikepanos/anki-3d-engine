#include "ModelNode.h"
#include "Model.h"
#include "Skeleton.h"
#include "ModelNodePatch.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void ModelNode::init(const char* filename)
{
	model.loadRsrc(filename);

	if(model->hasSkeleton())
	{
		heads.resize(model->getSkeleton().bones.size());
		tails.resize(model->getSkeleton().bones.size());
		boneRotations.resize(model->getSkeleton().bones.size());
		boneTranslations.resize(model->getSkeleton().bones.size());

	}

	for(uint i = 0; i < model->getModelPatches().size(); i++)
	{
		patches.push_back(new ModelNodePatch(model->getModelPatches()[i], model->hasSkeleton()));
	}
}
