// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/PlayerControllerComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

ANKI_SCENE_COMPONENT_STATICS(PlayerControllerComponent)

PlayerControllerComponent::PlayerControllerComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
{
	PhysicsPlayerControllerInitInfo init;
	init.m_position = Vec4(0.0f);
	m_player = node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsPlayerController>(init);
	m_player->setUserData(this);
}

} // end namespace anki