#include "anki/scene/SceneNode.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Movable.h"


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


//==============================================================================
void SceneNode::frameUpdate(float /*prevUpdateTime*/, float /*crntTime*/,
	int /*frame*/)
{
	// Movable update
	Movable* m = getMovable();
	if(m)
	{
		m->update();
	}
}


} // end namespace
