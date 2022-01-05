// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Skybox properties node.
class SkyboxNode : public SceneNode
{
public:
	SkyboxNode(SceneGraph* scene, CString name);

	~SkyboxNode();
};
/// @}

} // end namespace anki
