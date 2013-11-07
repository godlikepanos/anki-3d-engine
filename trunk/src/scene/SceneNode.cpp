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
		components(getSceneAllocator()),
		markedForDeletion(false)
{
	ANKI_ASSERT(scene);

	components.reserve(2);

	if(name_)
	{
		name = SceneString(name_, scene->getAllocator());
	}
}

//==============================================================================
SceneNode::~SceneNode()
{
	SceneAllocator<SceneComponent*> alloc = getSceneAllocator();
	for(auto comp : components)
	{
		alloc.deleteInstance(comp);
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
U32 SceneNode::getLastUpdateFrame() const
{
	U32 max = 0;

	iterateComponents([&](const SceneComponent& comp)
	{
		max = std::max(max, comp.getTimestamp());
	});

	return max;
}

} // end namespace anki
