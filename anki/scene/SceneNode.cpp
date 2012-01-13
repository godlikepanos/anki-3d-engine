#include "anki/scene/SceneNode.h"
#include "anki/scene/Scene.h"


namespace anki {


//==============================================================================
SceneNode::SceneNode(const char* name, Scene* scene)
{
	scene->registerNode(this);
}


//==============================================================================
SceneNode::~SceneNode()
{
	scene->unregisterNode(this);
}


} // end namespace
