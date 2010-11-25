#include "ModelNode.h"
#include "Model.h"
#include "Skeleton.h"


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
}
