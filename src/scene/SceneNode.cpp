#include "anki/scene/SceneNode.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Frustumable.h"

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

//==============================================================================
uint32_t SceneNode::getLastUpdateFrame()
{
	uint32_t max = 0;

	const Movable* m = getMovable();
	if(m && m->getMovableLastUpdateFrame() > max)
	{
		max = m->getMovableLastUpdateFrame();
	}

	const Frustumable* fr = getFrustumable();
	if(fr && fr->getFrustumableLastUpdateFrame() > max)
	{
		max = fr->getFrustumableLastUpdateFrame();
	}

	const Spatial* s = getSpatial();
	if(s && s->getSpatialLastUpdateFrame() > max)
	{
		max = s->getSpatialLastUpdateFrame();
	}

	return max;
}

} // end namespace anki
