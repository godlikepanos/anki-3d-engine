// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/PlayerControllerComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

PlayerControllerComponent::PlayerControllerComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
{
	PhysicsPlayerControllerInitInfo init;
	init.m_position = Vec3(0.0f);
	m_player = getExternalSubsystems(*node).m_physicsWorld->newInstance<PhysicsPlayerController>(init);
	m_player->setUserData(this);
}

} // end namespace anki
