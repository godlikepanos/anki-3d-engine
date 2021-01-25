// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/physics/Forward.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Trigger node.
class TriggerNode : public SceneNode
{
public:
	TriggerNode(SceneGraph* scene, CString name);

	~TriggerNode();

private:
	class MoveFeedbackComponent;
};
/// @}

} // end namespace anki
