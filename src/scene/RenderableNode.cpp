#include "RenderableNode.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
RenderableNode::RenderableNode(bool inheritParentTrfFlag,
	SceneNode* parent)
:	SceneNode(SNT_RENDERABLE, inheritParentTrfFlag, parent)
{}


RenderableNode::~RenderableNode()
{}
