#include <boost/foreach.hpp>
#include "SkinNode.h"
#include "Skin.h"
#include "SkinPatchNode.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void SkinNode::init(const char* filename)
{
	skin.loadRsrc(filename);

	BOOST_FOREACH(const ModelPatch& patch, skin->getModelPatches())
	{
		patches.push_back(new SkinPatchNode(patch, this));
	}
}


//======================================================================================================================
// moveUpdate                                                                                                          =
//======================================================================================================================
void SkinNode::moveUpdate()
{
	visibilityShapeWSpace.set(tails);
	visibilityShapeWSpace = visibilityShapeWSpace.getTransformed(getWorldTransform());
}
