// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Math.h>

namespace anki {

/// @addtogroup scene
/// @{

/// An event for simple movable animations
class JitterMoveEvent : public Event
{
public:
	JitterMoveEvent(Second startTime, Second duration, SceneNode* movableSceneNode);

	void update(Second prevUpdateTime, Second crntTime) override;

	void setPositionLimits(Vec3 posMin, Vec3 posMax);

private:
	Vec3 m_originalPos;
	Vec3 m_newPos;
};
/// @}

} // end namespace anki
