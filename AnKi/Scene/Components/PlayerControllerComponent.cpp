// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/PlayerControllerComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

PlayerControllerComponent::PlayerControllerComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
	PhysicsPlayerControllerInitInfo pinit;
	pinit.m_initialPosition = init.m_node->getWorldTransform().getOrigin().xyz;
	m_player = PhysicsWorld::getSingleton().newPlayerController(pinit);
	m_player->setUserData(this);

	init.m_node->setIgnoreParentTransform(true);
}

void PlayerControllerComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!info.m_node->isLocalTransformDirty())
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
	else
	{
		updated = true;
		m_player->moveToPosition(info.m_node->getLocalOrigin());
	}
}

} // end namespace anki
