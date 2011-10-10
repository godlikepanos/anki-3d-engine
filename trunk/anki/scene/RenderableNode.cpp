#include "anki/scene/RenderableNode.h"


RenderableNode::RenderableNode(Scene& scene, ulong flags, SceneNode* parent)
:	SceneNode(SNT_RENDERABLE_NODE, scene, flags, parent)
{}


RenderableNode::~RenderableNode()
{}
