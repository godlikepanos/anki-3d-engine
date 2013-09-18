#include "anki/scene/SceneNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Frustumable.h"

namespace anki {

//==============================================================================
SceneNode::SceneNode(const char* name_, SceneGraph* scene_, SceneNode* parent)
	:	Base(parent, scene_->getAllocator()),
		scene(scene_),
		name(getSceneAllocator()),
		markedForDeletion(false)
{
	ANKI_ASSERT(scene);

	if(name_)
	{
		name = SceneString(name_, scene->getAllocator());
	}
	
	/// Add the first property
	//addNewProperty(new ReadPointerProperty<std::string>("name", &name));
}

//==============================================================================
SceneNode::~SceneNode()
{}

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
U32 SceneNode::getLastUpdateFrame() const
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
