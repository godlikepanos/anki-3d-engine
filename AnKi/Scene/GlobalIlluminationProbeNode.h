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

/// Probe used in global illumination.
class GlobalIlluminationProbeNode : public SceneNode
{
public:
	GlobalIlluminationProbeNode(SceneGraph* scene, CString name);

	~GlobalIlluminationProbeNode();

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class MoveFeedbackComponent;
	class ShapeFeedbackComponent;

	Array<Transform, 6> m_cubeFaceTransforms;

	void onMoveUpdate(MoveComponent& move);
	void onShapeUpdateOrProbeNeedsRendering();
};
/// @}

} // end namespace anki
