// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Do visibility tests.
void doVisibilityTests(SceneNode& frustumable, SceneGraph& scene, RenderQueue& rqueue);
/// @}

} // end namespace anki
