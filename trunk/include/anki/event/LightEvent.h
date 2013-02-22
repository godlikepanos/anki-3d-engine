#ifndef ANKI_EVENT_LIGHT_EVENT_H
#define ANKI_EVENT_LIGHT_EVENT_H

#include "anki/event/Event.h"

namespace anki {

/// @addtogroup event
/// @{

/// Light event
class LightEvent: public Event
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// Constructor
	LightEvent(F32 startTime, F32 duration, EventManager* manager,
		const std::smart_ptr<SceneNode>& lightSn,
		const F32* radiusFrom, const F32* radiusTo);
	/// @}
};

/// @}

} // end namespace anki

#endif
