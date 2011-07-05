#include <boost/foreach.hpp>
#include "SkinNode.h"
#include "Resources/Skin.h"
#include "SkinPatchNode.h"
#include "Resources/Skeleton.h"


//==============================================================================
// init                                                                        =
//==============================================================================
void SkinNode::init(const char* filename)
{
	skin.loadRsrc(filename);

	BOOST_FOREACH(const ModelPatch& patch, skin->getModelPatches())
	{
		patches.push_back(new SkinPatchNode(patch, this));
	}

	uint bonesNum = skin->getSkeleton().getBones().size();
	tails.resize(bonesNum);
	heads.resize(bonesNum);
	boneRotations.resize(bonesNum);
	boneTranslations.resize(bonesNum);
}


//==============================================================================
// moveUpdate                                                                  =
//==============================================================================
void SkinNode::moveUpdate()
{
	visibilityShapeWSpace.set(tails);
	visibilityShapeWSpace = visibilityShapeWSpace.getTransformed(
		getWorldTransform());
}
