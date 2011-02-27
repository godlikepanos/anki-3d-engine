#include "RenderableNode.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
RenderableNode::RenderableNode(const Obb& visibilityShapeLSpace_, SceneNode* parent):
	SceneNode(SNT_RENDERABLE, false, parent),
	visibilityShapeLSpace(visibilityShapeLSpace_)
{}


//======================================================================================================================
// moveUpdate                                                                                                          =
//======================================================================================================================
void RenderableNode::moveUpdate()
{
	visibilityShapeWSpace = visibilityShapeLSpace.getTransformed(getWorldTransform());
}
