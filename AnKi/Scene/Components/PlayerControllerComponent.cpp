// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/PlayerControllerComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics2/PhysicsWorld.h>

namespace anki {

PlayerControllerComponent::PlayerControllerComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	v2::PhysicsPlayerControllerInitInfo init;
	init.m_initialPosition = node->getWorldTransform().getOrigin().xyz();
	m_player = v2::PhysicsWorld::getSingleton().newPlayerController(init);
	m_player->setUserData(this);

	node->setIgnoreParentTransform(true);
}

Error PlayerControllerComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	U32 posVersion;
	const Vec3 newPos = m_player->getPosition(&posVersion);

	if(posVersion != m_positionVersion)
	{
		updated = true;
		m_positionVersion = posVersion;

		info.m_node->setLocalOrigin(newPos.xyz0());
	}

	return Error::kNone;
}

} // end namespace anki
