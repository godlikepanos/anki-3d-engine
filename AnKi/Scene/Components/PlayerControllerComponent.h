// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics/PhysicsPlayerController.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Physics player controller component.
class PlayerControllerComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(PlayerControllerComponent)

public:
	PlayerControllerComponent(SceneNode* node);

	void setVelocity(F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed, const Vec4& forwardDir)
	{
		m_player->setVelocity(forwardSpeed, strafeSpeed, jumpSpeed, forwardDir);
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
	Vec3 m_worldPos = Vec3(0.0f);

	Error update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated);
};
/// @}

} // end namespace anki
