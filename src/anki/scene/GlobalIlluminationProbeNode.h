// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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

/// Probe used in global illumination.
class GlobalIlluminationProbeNode : public SceneNode
{
public:
	GlobalIlluminationProbeNode(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
	}

	~GlobalIlluminationProbeNode();

	ANKI_USE_RESULT Error init();

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class MoveFeedbackComponent;

	Array<Transform, 6> m_cubeFaceTransforms;
	Aabb m_spatialAabb;

	void onMoveUpdate(MoveComponent& move);
};
/// @}

} // end namespace anki
