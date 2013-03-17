#ifndef ANKI_EVENT_LIGHT_EVENT_H
#define ANKI_EVENT_LIGHT_EVENT_H

#include "anki/event/Event.h"
#include "anki/math/Math.h"

namespace anki {

// Forward
class Light;

/// Helper class
struct LightEventData
{
	Light* light = nullptr;
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
	LightEvent(F32 startTime, F32 duration, EventManager* manager,
		U8 flags, const LightEventData& data);
	/// @}

	/// Implements Event::update
	void update(F32 prevUpdateTime, F32 crntTime);

private:
	F32 originalRadius;
	Vec4 originalDiffColor;
	Vec4 originalSpecColor;
};

} // end namespace anki

#endif
