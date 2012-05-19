#include "anki/scene/SceneNode.h"
#include "anki/scene/Scene.h"


namespace anki {


//==============================================================================
SceneNode::SceneNode(const char* name_, Scene* scene_)
	: name(name_), scene(scene_)
{
	scene->registerNode(this);

	/// Add the first property
	addNewProperty(new ReadPointerProperty<std::string>("name", &name));
}


//==============================================================================
SceneNode::~SceneNode()
{
	scene->unregisterNode(this);
}


} // end namespace
