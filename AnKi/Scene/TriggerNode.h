// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Physics/Forward.h>

namespace anki {

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
