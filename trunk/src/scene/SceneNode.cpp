#include "anki/scene/SceneNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/FrustumComponent.h"

namespace anki {

//==============================================================================
SceneNode::SceneNode(const char* name_, SceneGraph* scene_)
	:	SceneObject(nullptr, scene_),
		name(getSceneAllocator()),
		components(getSceneAllocator())
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
U32 SceneNode::getLastUpdateFrame() const
{
	U32 max = 0;

	iterateComponents([&](const SceneComponent& comp)
	{
		max = std::max(max, comp.getTimestamp());
	});

	return max;
}

//==============================================================================
void SceneNode::addComponent(SceneComponent* comp)
{
	ANKI_ASSERT(comp);
	components.push_back(comp);
}

//==============================================================================
void SceneNode::removeComponent(SceneComponent* comp)
{
	auto it = std::find(components.begin(), components.end(), comp);
	ANKI_ASSERT(it != components.end());
	components.erase(it);
}

} // end namespace anki
