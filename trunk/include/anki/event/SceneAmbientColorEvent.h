#ifndef ANKI_EVENT_SCENE_AMBIENT_COLOR_EVENT_H
#define ANKI_EVENT_SCENE_AMBIENT_COLOR_EVENT_H

#include "anki/event/Event.h"
#include "anki/Math.h"

namespace anki {

// Forward
class SceneGraph;

/// Change the scene color
class SceneAmbientColorEvent: public Event
{
public:
	/// Constructor
	SceneAmbientColorEvent(F32 startTime, F32 duration, EventManager* manager,
		const Vec4& finalColor, SceneGraph* scene);

	/// Implements Event::update
	void update(F32 prevUpdateTime, F32 crntTime);

private:
	Vec4 originalColor; ///< Original scene color. The constructor sets it
	Vec4 finalColor;
	SceneGraph* scene = nullptr;
};

} // end namespace anki

#endif
