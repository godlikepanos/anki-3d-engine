#include "anki/scene/SceneNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/FrustumComponent.h"

namespace anki {

//==============================================================================
SceneNode::SceneNode(const char* name_, SceneGraph* scene_)
	:	scene(scene_),
		name(getSceneAllocator()),
		markedForDeletion(false)
{
	ANKI_ASSERT(scene);

	if(name_)
	{
		name = SceneString(name_, scene->getAllocator());
	}
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

	const MoveComponent* m = getMoveComponent();
	if(m && m->getTimestamp() > max)
	{
		max = m->getTimestamp();
	}

	const FrustumComponent* fr = getFrustumComponent();
	if(fr && fr->getTimestamp() > max)
	{
		max = fr->getTimestamp();
	}

	const SpatialComponent* s = getSpatialComponent();
	if(s && s->getTimestamp() > max)
	{
		max = s->getTimestamp();
	}

	return max;
}

} // end namespace anki
