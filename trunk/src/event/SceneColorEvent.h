#ifndef SCENE_COLOR_EVENT_H
#define SCENE_COLOR_EVENT_H

#include "Event.h"
#include "math/Math.h"


/// Change the scene color
class SceneColorEvent: public Event
{
	public:
		/// Constructor
		SceneColorEvent(float startTime, float duration,
			const Vec3& finalColor);

		/// Copy constructor
		SceneColorEvent(const SceneColorEvent& b);

		/// Copy
		SceneColorEvent& operator=(const SceneColorEvent& b);

	private:
		Vec3 originalColor; ///< Original scene color. The constructor sets it
		Vec3 finalColor;

		/// Implements Event::updateSp
		void updateSp(float prevUpdateTime, float crntTime);
};


#endif
