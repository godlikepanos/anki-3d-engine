#include "anki/scene/RenderableNode.h"


namespace anki {


RenderableNode::RenderableNode(Scene& scene, ulong flags, SceneNode* parent)
:	SceneNode(SNT_RENDERABLE_NODE, scene, flags, parent)
{}


RenderableNode::~RenderableNode()
{}


} // end namespace
