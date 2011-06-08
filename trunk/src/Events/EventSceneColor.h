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
		SceneColor(uint startTime, int duration, const Vec3& finalColor);

		/// Copy constructor
		SceneColor(const SceneColor& b) {*this = b;}

		/// Copy
		SceneColor& operator=(const SceneColor& b);

		/// Implements Event::updateSp
		void updateSp(uint timeUpdate);

	private:
		Vec3 finalColor;
};


} // end namespace

#endif
