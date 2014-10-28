// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_INSTANCE_NODE_H
#define ANKI_SCENE_INSTANCE_NODE_H

#include "anki/scene/SceneNode.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Instance component. Dummy used only for idendification
class InstanceComponent: public SceneComponent
{
public:
	InstanceComponent(SceneNode* node)
	:	SceneComponent(Type::INSTANCE, node)
	{}

	static constexpr Type getClassType()
	{
		return Type::INSTANCE;
	}
};

/// Instance scene node
class InstanceNode: public SceneNode,  public MoveComponent, 
	public InstanceComponent
{
public:
	InstanceNode(SceneGraph* scene)
	:	SceneNode(scene), 
		MoveComponent(this, MoveComponent::Flag::IGNORE_PARENT_TRANSFORM),
		InstanceComponent(this)
	{}

	ANKI_USE_RESULT Error create(const CString& name)
	{
		Error err = SceneNode::create(name, 2);

		if(!err)
		{
			addComponent(static_cast<MoveComponent*>(this));
			addComponent(static_cast<InstanceComponent*>(this));
		}

		return err;
	}
};

/// @}

} // end namespace anki

#endif

