// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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

	const Transform& getWorldTransform() const
	{
		return m_trf;
	}

	void setWorldTransform(const Transform& trf)
	{
		m_player->moveToPosition(trf.getOrigin().xyz());
	}

	void setVelocity(F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed, const Vec4& forwardDir)
	{
		m_player->setVelocity(forwardSpeed, strafeSpeed, jumpSpeed, forwardDir);
	}

	void moveToPosition(const Vec3& pos)
	{
		m_player->moveToPosition(pos);
	}

	Error update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated) override
	{
		const Transform newTrf = m_player->getTransform();
		updated = newTrf != m_trf;
		m_trf = newTrf;
		return Error::kNone;
	}

	PhysicsPlayerControllerPtr getPhysicsPlayerController() const
	{
		return m_player;
	}

private:
	PhysicsPlayerControllerPtr m_player;
	Transform m_trf = Transform::getIdentity();
};
/// @}

} // end namespace anki
