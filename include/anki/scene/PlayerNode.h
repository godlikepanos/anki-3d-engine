// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_PLAYER_NODE_H
#define ANKI_SCENE_PLAYER_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/Math.h"

namespace anki {

// Forward
class PhysicsPlayerController;

/// @addtogroup scene
/// @{

/// Player scene node. It uses input and physics to move inside the world.
class PlayerNode: public SceneNode
{
	friend class PlayerNodeFeedbackComponent;

public:
	PlayerNode(SceneGraph* scene);

	~PlayerNode();

	ANKI_USE_RESULT Error create(const CString& name, const Vec4& position);

private:
	PhysicsPlayerController* m_player = nullptr;
};
/// @}

} // end namespace anki

#endif

