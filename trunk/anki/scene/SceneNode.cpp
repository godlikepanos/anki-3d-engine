#include "anki/scene/SceneNode.h"
#include "anki/scene/Scene.h"


namespace anki {


//==============================================================================
SceneNode::SceneNode(const char* name, Scene* scene)
{
	scene->registerNode(this);

	/// Add the first property
	pmap.addProperty("name", &name, PropertyBase::PF_READ);
}


//==============================================================================
SceneNode::~SceneNode()
{
	scene->unregisterNode(this);
}


} // end namespace
