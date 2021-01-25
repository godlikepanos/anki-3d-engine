// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/collision/Aabb.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Fog density node.
class FogDensityNode : public SceneNode
{
public:
	FogDensityNode(SceneGraph* scene, CString name);

	~FogDensityNode();

private:
	class MoveFeedbackComponent;
	class DensityShapeFeedbackComponent;

	void onMoveUpdated(const MoveComponent& movec);
	void onDensityShapeUpdated(const FogDensityComponent& fogc);
};
/// @}

} // end namespace anki
