#ifndef ANKI_EVENT_EVENT_H
#define ANKI_EVENT_EVENT_H

namespace anki {

/// Abstract class for all events. All Event derived classes should be copy-able
/// In order to recycle the events and save the mallocs
class Event
{
public:
	/// The event type enum
	enum EventType
	{
		ET_SCENE_COLOR,
		ET_MAIN_RENDERER_PPS_HDR,
		ET_COUNT
	};

	/// Constructor
	Event(EventType type, float startTime, float duration);

	/// Copy constructor
	Event(const Event& b)
	{
		*this = b;
	}

	/// @name Accessors
	/// @{
	float getStartTime() const
	{
		return startTime;
	}

	float getDuration() const
	{
		return duration;
	}

	EventType getEventType() const
	{
		return type;
	}

	bool isDead(float crntTime) const
	{
		return crntTime >= startTime + duration;
	}
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
	static Type interpolate(const Type& from, const Type& to, float delta)
	{
		return from * (1.0 - delta) + to * delta;
	}

private:
	EventType type; ///< Self explanatory
	float startTime; ///< The time the event will start. Eg 23:00
	float duration; ///< The duration of the event
};

} // end namespace

#endif
