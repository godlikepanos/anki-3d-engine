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
		bool isDead(uint crntTime) const {return crntTime >= startTime + duration;}
		/// @}

		/// Copy
		Event& operator=(const Event& b);

		/// @param[in] prevUpdateTime The time of the previous update (ms)
		/// @param[in] crntTime The current time (ms)
		void update(uint prevUpdateTime, uint crntTime);

	protected:
		virtual void updateSp(uint prevUpdateTime, uint crntTime) = 0;

	private:
		EventType type; ///< Self explanatory
		uint startTime; ///< The time the event will start. Eg 23:00
		int duration; ///< The duration of the event
};


} // end namespace


#endif
