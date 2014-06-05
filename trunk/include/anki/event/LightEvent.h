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

/// @addtogroup Events
/// @{

/// Helper class
struct LightEventData
{
	F32 radiusMultiplier = 0.0;
	Vec4 intensityMultiplier = Vec4(0.0);
	Vec4 specularIntensityMultiplier = Vec4(0.0);
};

/// An event for light animation
class LightEvent: public Event, private LightEventData
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// Constructor
	LightEvent(EventManager* manager, F32 startTime, F32 duration,
		Light* light, const LightEventData& data);
	/// @}

	/// Implements Event::update
	void update(F32 prevUpdateTime, F32 crntTime);

private:
	Light* light;
	F32 originalRadius;
	Vec4 originalDiffColor;
	Vec4 originalSpecColor;
};
/// @}

} // end namespace anki

#endif
