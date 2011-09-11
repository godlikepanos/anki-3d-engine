#include "RenderableNode.h"


RenderableNode::RenderableNode(ClassId cid, Scene& scene, ulong flags,
	SceneNode* parent)
:	SceneNode(cid, scene, flags, parent)
{}


RenderableNode::~RenderableNode()
{}
