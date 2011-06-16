#ifndef EVENT_H
#define EVENT_H

#include "StdTypes.h"
#include "Accessors.h"


namespace Event {


/// The event type enum
enum EventType
{
	SCENE_COLOR,
	MAIN_RENDERER_PPS_HDR,
	EVENT_TYPES_NUM
};


/// Abstract class for all events. All Event derived classes should be copy-able
class Event
{
	public:
		/// Constructor
		Event(EventType type, uint startTime, uint duration);

		/// Copy constructor
		Event(const Event& b) {*this = b;}

		/// @name Accessors
		/// @{
		GETTER_R(uint, startTime, getStartTime)
		GETTER_R(uint, duration, getDuration)
		GETTER_R(EventType, type, getEventType)
		bool isDead(uint crntTime) const {return crntTime >= startTime + duration;}
		/// @}

		/// Copy
		Event& operator=(const Event& b);

		/// @param[in] prevUpdateTime The time of the previous update (ms)
		/// @param[in] crntTime The current time (ms)
		void update(uint prevUpdateTime, uint crntTime);

	protected:
		/// This method should be implemented by the derived classes
		virtual void updateSp(uint prevUpdateTime, uint crntTime) = 0;

		/// Linear interpolation between values
		/// @param[in] from Starting value
		/// @param[in] to Ending value
		/// @param[in] delta The percentage from the from "from" value. Values from [0.0, 1.0]
		template<typename Type>
		static Type interpolate(const Type& from, const Type& to, float delta);

	private:
		EventType type; ///< Self explanatory
		uint startTime; ///< The time the event will start. Eg 23:00
		uint duration; ///< The duration of the event
};


template<typename Type>
Type Event::interpolate(const Type& from, const Type& to, float delta)
{
	return from * (1.0 - delta) + to * delta;
}


} // end namespace


#endif
