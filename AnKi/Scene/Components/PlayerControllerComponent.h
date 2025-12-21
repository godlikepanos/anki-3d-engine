// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics/PhysicsPlayerController.h>

namespace anki {

// Physics player controller component.
class PlayerControllerComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(PlayerControllerComponent)

public:
	PlayerControllerComponent(SceneNode* node, U32 uuid);

	void setVelocity(F32 forwardSpeed, F32 jumpSpeed, Vec3 forwardDir, Bool crouch)
	{
		m_player->updateState(forwardSpeed, forwardDir, jumpSpeed, crouch);
	}

	void moveToPosition(const Vec3& pos)
	{
		m_player->moveToPosition(pos);
	}

	PhysicsPlayerController& getPhysicsPlayerController()
	{
		return *m_player;
	}

private:
	PhysicsPlayerControllerPtr m_player;
	U32 m_positionVersion = kMaxU32;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;
};

} // end namespace anki
