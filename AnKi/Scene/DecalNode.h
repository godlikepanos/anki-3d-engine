// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Node that has a decal component.
class DecalNode : public SceneNode
{
public:
	DecalNode(SceneGraph* scene, CString name);

	~DecalNode();

private:
	class MoveFeedbackComponent;
	class ShapeFeedbackComponent;

	void onMove(MoveComponent& movec);
	void onDecalUpdated(DecalComponent& decalc);
};
/// @}

} // end namespace anki
