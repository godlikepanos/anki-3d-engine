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
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::PLAYER_CONTROLLER;

	PlayerControllerComponent(PhysicsPlayerControllerPtr player)
		: SceneComponent(CLASS_TYPE)
		, m_player(player)
	{
	}

	~PlayerControllerComponent() = default;

	const Transform& getTransform() const
	{
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		m_player->moveToPosition(trf.getOrigin());
	}

	void setVelocity(F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed, const Vec4& forwardDir)
	{
		m_player->setVelocity(forwardSpeed, strafeSpeed, jumpSpeed, forwardDir);
	}

	void moveToPosition(Vec4 pos)
	{
		m_player->moveToPosition(pos);
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
	Transform m_trf;
};
/// @}

} // end namespace anki
