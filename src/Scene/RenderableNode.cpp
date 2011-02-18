#include "RenderableNode.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
RenderableNode::RenderableNode(const Sphere& boundingShapeLSpace_, SceneNode* parent):
	SceneNode(SNT_RENDERABLE, false, parent),
	boundingShapeLSpace(boundingShapeLSpace_)
{}


//======================================================================================================================
// moveUpdate                                                                                                          =
//======================================================================================================================
void RenderableNode::moveUpdate()
{
	boundingShapeWSpace = boundingShapeLSpace.getTransformed(getWorldTransform());
}
