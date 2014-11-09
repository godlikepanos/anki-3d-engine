// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_EVENT_LIGHT_EVENT_H
#define ANKI_EVENT_LIGHT_EVENT_H

#include "anki/event/Event.h"
#include "anki/Math.h"

namespace anki {

// Forward
class Light;

/// @addtogroup event
/// @{

/// Helper class
class LightEventData
{
public:
	F32 m_radiusMultiplier = 0.0;
	Vec4 m_intensityMultiplier = Vec4(0.0);
	Vec4 m_specularIntensityMultiplier = Vec4(0.0);
};

/// An event for light animation
class LightEvent: public Event, private LightEventData
{
public:
	/// Create
	ANKI_USE_RESULT Error create(
		EventManager* manager, F32 startTime, F32 duration,
		Light* light, const LightEventData& data);

	/// Implements Event::update
	ANKI_USE_RESULT Error update(F32 prevUpdateTime, F32 crntTime);

private:
	F32 m_originalRadius;
	Vec4 m_originalDiffColor;
	Vec4 m_originalSpecColor;
};
/// @}

} // end namespace anki

#endif
