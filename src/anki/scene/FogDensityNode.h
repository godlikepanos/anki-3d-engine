// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

	ANKI_USE_RESULT Error init()
	{
		return Error::NONE;
	}

private:
	class FeedbackComponent;

	Aabb m_spatialBox;

	void moveUpdated(const MoveComponent& movec);
};
/// @}

} // end namespace anki