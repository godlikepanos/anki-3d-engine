#ifndef ANKI_SCENE_INSTANCE_NODE_H
#define ANKI_SCENE_INSTANCE_NODE_H

#include "anki/scene/SceneNode.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Instance component. Dummy used only for idendification
class InstanceComponent: public SceneComponent
{
public:
	InstanceComponent(SceneNode* node)
		: SceneComponent(this, node)
	{}
};

/// Instance scene node
class InstanceNode: public SceneNode, public InstanceComponent, 
	public MoveComponent
{
public:
	InstanceNode(const char* name, SceneGraph* scene)
		: SceneNode(name, scene), InstanceComponent(this), MoveComponent(this)
	{
		addComponent(static_cast<InstanceComponent*>(this));
		addComponent(static_cast<MoveComponent*>(this));
	}
};

/// @}

} // end namespace anki

#endif

