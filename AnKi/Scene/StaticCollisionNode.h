// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Physics/Forward.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Node that interacts with physics.
class StaticCollisionNode : public SceneNode
{
public:
	StaticCollisionNode(SceneGraph* scene, CString name);

	~StaticCollisionNode();
};
/// @}

} // end namespace anki
