#include "anki/scene/SceneNode.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Frustumable.h"

namespace anki {

//==============================================================================
SceneNode::SceneNode(const char* name_, Scene* scene_)
	: name(name_, scene_->getAllocator()), scene(scene_)
{
	name.shrink_to_fit(); // Do that first
	scene->registerNode(this);

	/// Add the first property
	//addNewProperty(new ReadPointerProperty<std::string>("name", &name));
}

//==============================================================================
SceneNode::~SceneNode()
{
	scene->unregisterNode(this);

	if(getSpatial() && getSpatial()->octreeNode)
	{
		getSpatial()->octreeNode->removeSceneNode(this);
	}
}

//==============================================================================
SceneAllocator<U8> SceneNode::getSceneAllocator() const
{
	ANKI_ASSERT(scene);
	return scene->getAllocator();
}

//==============================================================================
SceneAllocator<U8> SceneNode::getSceneFrameAllocator() const
{
	ANKI_ASSERT(scene);
	return scene->getFrameAllocator();
}

//==============================================================================
U32 SceneNode::getLastUpdateFrame()
{
	U32 max = 0;

	const Movable* m = getMovable();
	if(m && m->getMovableTimestamp() > max)
	{
		max = m->getMovableTimestamp();
	}

	const Frustumable* fr = getFrustumable();
	if(fr && fr->getFrustumableTimestamp() > max)
	{
		max = fr->getFrustumableTimestamp();
	}

	const Spatial* s = getSpatial();
	if(s && s->getSpatialTimestamp() > max)
	{
		max = s->getSpatialTimestamp();
	}

	return max;
}

} // end namespace anki
