// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/scene/SceneComponent.h"
#include "anki/physics/PhysicsPlayerController.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Physics player controller component.
class PlayerControllerComponent: public SceneComponent
{
public:
	PlayerControllerComponent(SceneNode* node,
		PhysicsPlayerControllerPtr player)
		: SceneComponent(Type::PLAYER_CONTROLLER, node)
		, m_player(player)
	{}

	~PlayerControllerComponent() = default;

	const Transform& getTransform() const
	{
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		m_player->moveToPosition(trf.getOrigin());
	}

	void setVelocity(F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed,
		const Vec4& forwardDir)
	{
		m_player->setVelocity(forwardSpeed, strafeSpeed, jumpSpeed, forwardDir);
	}

	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override
	{
		m_trf = m_player->getTransform(updated);
		return ErrorCode::NONE;
	}

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::PLAYER_CONTROLLER;
	}

private:
	PhysicsPlayerControllerPtr m_player;
	Transform m_trf;
};
/// @}

} // end namespace anki

