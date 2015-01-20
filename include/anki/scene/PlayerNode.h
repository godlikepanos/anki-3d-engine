// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_PLAYER_NODE_H
#define ANKI_SCENE_PLAYER_NODE_H

#include "anki/scene/SceneNode.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Player scene node. It uses input and physics to move inside the world.
class PlayerNode: public SceneNode
{
public:
	PlayerNode(SceneGraph* scene);
};
/// @}

} // end namespace anki

#endif

