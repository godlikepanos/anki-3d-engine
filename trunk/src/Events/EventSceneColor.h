#ifndef EVENT_SCENE_COLOR_H
#define EVENT_SCENE_COLOR_H

#include "Event.h"
#include "Math.h"


namespace Event {


/// Change the scene color
class SceneColor: public Event
{
	public:
		/// Constructor
		SceneColor(uint startTime, uint duration, const Vec3& finalColor);

		/// Copy constructor
		SceneColor(const SceneColor& b);

		/// Copy
		SceneColor& operator=(const SceneColor& b);

	private:
		Vec3 originalColor; ///< Original scene color. The constructor sets it
		Vec3 finalColor;

		/// Implements Event::updateSp
		void updateSp(uint prevUpdateTime, uint crntTime);
};


} // end namespace

#endif
