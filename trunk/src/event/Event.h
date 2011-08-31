#ifndef EVENT_H
#define EVENT_H

#include "util/StdTypes.h"
#include "util/Accessors.h"


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
		Event(EventType type, float startTime, float duration);

		/// Copy constructor
		Event(const Event& b) {*this = b;}

		/// @name Accessors
		/// @{
		GETTER_R(float, startTime, getStartTime)
		GETTER_R(float, duration, getDuration)
		GETTER_R(EventType, type, getEventType)
		bool isDead(float crntTime) const;
		/// @}

		/// Copy
		Event& operator=(const Event& b);

		/// @param[in] prevUpdateTime The time of the previous update (sec)
		/// @param[in] crntTime The current time (sec)
		void update(float prevUpdateTime, float crntTime);

	protected:
		/// This method should be implemented by the derived classes
		virtual void updateSp(float prevUpdateTime, float crntTime) = 0;

		/// Linear interpolation between values
		/// @param[in] from Starting value
		/// @param[in] to Ending value
		/// @param[in] delta The percentage from the from "from" value. Values
		/// from [0.0, 1.0]
		template<typename Type>
		static Type interpolate(const Type& from, const Type& to, float delta);

	private:
		EventType type; ///< Self explanatory
		float startTime; ///< The time the event will start. Eg 23:00
		float duration; ///< The duration of the event
};


inline bool Event::isDead(float crntTime) const
{
	return crntTime >= startTime + duration;
}


template<typename Type>
Type Event::interpolate(const Type& from, const Type& to, float delta)
{
	return from * (1.0 - delta) + to * delta;
}


#endif
