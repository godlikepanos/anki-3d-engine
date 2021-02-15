// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Math.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Probe used in realtime reflections.
class ReflectionProbeNode : public SceneNode
{
public:
	ReflectionProbeNode(SceneGraph* scene, CString name);

	~ReflectionProbeNode();

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class MoveFeedbackComponent;
	class ShapeFeedbackComponent;

	Array<Transform, 6> m_frustumTransforms;

	void onMoveUpdate(MoveComponent& move);
	void onShapeUpdate(ReflectionProbeComponent& reflc);
};
/// @}

} // end namespace anki
