// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/Math.h>
#include <anki/physics/PhysicsPlayerController.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Player scene node. It uses input and physics to move inside the world.
class PlayerNode : public SceneNode
{
	friend class PlayerNodeFeedbackComponent;

public:
	PlayerNode(SceneGraph* scene, CString name);

	~PlayerNode();

	ANKI_USE_RESULT Error init(const Vec4& position);

private:
	PhysicsPlayerControllerPtr m_player;
};
/// @}

} // end namespace anki
