// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_PLAYER_CONTROLLER_COMPONENT_H
#define ANKI_SCENE_PLAYER_CONTROLLER_COMPONENT_H

#include "anki/scene/SceneComponent.h"
#include "anki/physics/PlayerControllerComponent.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Physics player controller component.
class PlayerControllerComponent: public SceneComponent
{
public:
	PlayerControllerComponent(SceneNode* node, PhysicsPlayerController* player)
	:	SceneComponent(Type::PLAYER_CONTROLLER, node),
		m_player(player)
	{}

	const Transform& getTransform() const
	{
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		m_player->moveToPosition(trf.getOrigin());
	}

	/// @name SceneComponent overrides
	/// @{
	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override
	{
		m_trf = m_player->getTransform(updated);
		return ErrorCode::NONE;
	}
	/// @}

	static constexpr Type getClassType()
	{
		return Type::PLAYER_CONTROLLER;
	}

private:
	PhysicsPlayerController* m_player;
	Transform m_trf;
};
/// @}

} // end namespace anki

#endif

