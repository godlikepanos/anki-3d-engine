// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/script/Common.h"
#include "anki/Scene.h"

namespace anki {

//==============================================================================
ANKI_SCRIPT_WRAP(MoveComponent)
{
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(lb, MoveComponent)
		ANKI_LUA_METHOD("setLocalOrigin", &MoveComponent::setLocalOrigin)
		ANKI_LUA_METHOD("getLocalOrigin", &MoveComponent::getLocalOrigin)
		ANKI_LUA_METHOD("setLocalRotation", &MoveComponent::setLocalRotation)
		ANKI_LUA_METHOD("getLocalRotation", &MoveComponent::getLocalRotation)
		ANKI_LUA_METHOD("setLocalScale", &MoveComponent::setLocalScale)
		ANKI_LUA_METHOD("getLocalScale", &MoveComponent::getLocalScale)
	ANKI_LUA_CLASS_END()
}

//==============================================================================
static void sceneNodeAddChild(SceneNode* node, SceneNode* child)
{
	node->addChild(child);
}

ANKI_SCRIPT_WRAP(SceneNode)
{
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(lb, SceneNode)
		ANKI_LUA_METHOD("getName", &SceneNode::getName)
		ANKI_LUA_FUNCTION_AS_METHOD("addChild", &sceneNodeAddChild);
		ANKI_LUA_METHOD_DETAIL("getMoveComponent", 
			MoveComponent& (SceneNode::*)(),
			&SceneNode::getComponent<MoveComponent>,
			BinderFlag::NONE)
	ANKI_LUA_CLASS_END()
}

//==============================================================================
static SceneNode* modelNodeGetSceneNodeBase(ModelNode* self)
{
	ANKI_ASSERT(self);
	return static_cast<SceneNode*>(self);
}

ANKI_SCRIPT_WRAP(ModelNode)
{
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(lb, ModelNode)
		ANKI_LUA_FUNCTION_AS_METHOD("getSceneNodeBase", 
			&modelNodeGetSceneNodeBase);
	ANKI_LUA_CLASS_END()
}

//==============================================================================
static SceneNode* instanceNodeGetSceneNodeBase(InstanceNode* self)
{
	ANKI_ASSERT(self);
	return static_cast<SceneNode*>(self);
}

ANKI_SCRIPT_WRAP(InstanceNode)
{
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(lb, InstanceNode)
		ANKI_LUA_FUNCTION_AS_METHOD("getSceneNodeBase", 
			&instanceNodeGetSceneNodeBase);
	ANKI_LUA_CLASS_END()
}

//==============================================================================
ANKI_SCRIPT_WRAP(SceneGraph)
{
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(lb, SceneGraph)
		ANKI_LUA_METHOD("newModelNode", 
			(&SceneGraph::newSceneNode<ModelNode, const char*>))
		ANKI_LUA_METHOD("newInstanceNode", 
			(&SceneGraph::newSceneNode<InstanceNode>))
		ANKI_LUA_METHOD("tryFindSceneNode", &SceneGraph::tryFindSceneNode)
	ANKI_LUA_CLASS_END()
}

ANKI_SCRIPT_WRAP_SINGLETON(SceneGraphSingleton)

} // end namespace anki
