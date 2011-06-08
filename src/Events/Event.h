#ifndef EVENT_H
#define EVENT_H

#include "StdTypes.h"
#include "Accessors.h"


namespace Event {


/// The event type enum
enum EventType
{
	SCENE_COLOR,
	EVENT_TYPES_NUM
};


/// Abstract class for all events
class Event
{
	public:
		/// Constructor
		Event(EventType type, uint startTime, int duration);

		/// Copy constructor
		Event(const Event& b) {*this = b;}

		/// @name Accessors
		/// @{
		GETTER_R(uint, startTime, getStartTime)
		GETTER_R(int, duration, getDuration)
		GETTER_R(EventType, type, getEventType)
		bool isFinished() const {return duration < 0;}
		/// @}

		/// Copy
		Event& operator=(const Event& b);

		/// @param[in] timeUpdate The time between this update and the previous
		void update(uint timeUpdate);
		virtual void updateSp(uint timeUpdate) = 0;

	private:
		EventType type; ///< Self explanatory
		uint startTime; ///< The time the event will start. Eg 23:00
		/// The duration of the event. < 0 means finished, 0 means no limit and > 0 is the actual duration
		int duration;
};


} // end namespace


#endif
