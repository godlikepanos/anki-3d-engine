// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/physics/PhysicsPlayerController.h>

namespace anki
{

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
		m_player->moveToPosition(trf.getOrigin());
	}

	void setVelocity(F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed, const Vec4& forwardDir)
	{
		m_player->setVelocity(forwardSpeed, strafeSpeed, jumpSpeed, forwardDir);
	}

	void moveToPosition(const Vec3& pos)
	{
		m_player->moveToPosition(pos.xyz0());
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		m_trf = m_player->getTransform(updated);
		return Error::NONE;
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
