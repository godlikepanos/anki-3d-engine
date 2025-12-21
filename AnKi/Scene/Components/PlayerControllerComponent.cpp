// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/PlayerControllerComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

PlayerControllerComponent::PlayerControllerComponent(SceneNode* node, U32 uuid)
	: SceneComponent(node, kClassType, uuid)
{
	PhysicsPlayerControllerInitInfo init;
	init.m_initialPosition = node->getWorldTransform().getOrigin().xyz;
	m_player = PhysicsWorld::getSingleton().newPlayerController(init);
	m_player->setUserData(this);

	node->setIgnoreParentTransform(true);
}

void PlayerControllerComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	U32 posVersion;
	const Vec3 newPos = m_player->getPosition(&posVersion);

	if(posVersion != m_positionVersion)
	{
		updated = true;
		m_positionVersion = posVersion;

		info.m_node->setLocalOrigin(newPos);
	}
}

} // end namespace anki
