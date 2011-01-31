#include "RenderableNode.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
RenderableNode::RenderableNode(const Sphere& boundingShapeLSpace_, SceneNode* parent):
	SceneNode(SNT_RENDERABLE, false, parent),
	boundingShapeLSpace(boundingShapeLSpace_)
{}


//======================================================================================================================
// updateTrf                                                                                                           =
//======================================================================================================================
void RenderableNode::updateTrf()
{
	boundingShapeWSpace = boundingShapeLSpace.getTransformed(getWorldTransform());
}
