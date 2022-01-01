// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Math.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Player scene node. It uses input and physics to move inside the world.
class PlayerNode : public SceneNode
{
public:
	PlayerNode(SceneGraph* scene, CString name);

	~PlayerNode();

private:
	class FeedbackComponent;
	class FeedbackComponent2;
};
/// @}

} // end namespace anki
