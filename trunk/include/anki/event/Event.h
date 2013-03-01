#ifndef ANKI_EVENT_EVENT_H
#define ANKI_EVENT_EVENT_H

#include "anki/scene/Common.h"
#include "anki/util/Bitset.h"

namespace anki {

// Forward
class EventManager;

/// Abstract class for all events
class Event: public Bitset<U8>
{
	friend class EventManager;

public:
	/// Event flags
	enum EventFlags
	{
		EF_NONE = 0,
		EF_REANIMATE = 1 << 0
	};

	/// @name Constructors/Destructor
	/// @{

	/// Constructor
	Event(F32 startTime, F32 duration, EventManager* manager, 
		U8 flags = EF_NONE);

	virtual ~Event();
	/// @}

	/// @name Accessors
	/// @{
	F32 getStartTime() const
	{
		return startTime;
	}

	F32 getDuration() const
	{
		return duration;
	}

	Bool isDead(F32 crntTime) const
	{
		return crntTime >= startTime + duration;
	}

	SceneAllocator<U8> getSceneAllocator() const;
	SceneAllocator<U8> getSceneFrameAllocator() const;
	/// @}

	/// This method should be implemented by the derived classes
	/// @param[in] prevUpdateTime The time of the previous update (sec)
	/// @param[in] crntTime The current time (sec)
	virtual void update(F32 prevUpdateTime, F32 crntTime) = 0;

protected:
	/// Linear interpolation between values
	/// @param[in] from Starting value
	/// @param[in] to Ending value
	/// @param[in] delta The percentage from the from "from" value. Values
	///                  from [0.0, 1.0]
	template<typename Type>
	static Type interpolate(const Type& from, const Type& to, F32 delta)
	{
		ANKI_ASSERT(delta >= 0 && delta <= 1.0);
		return from * (1.0 - delta) + to * delta;
	}

	/// Return the delta between current time and when the event started
	/// @return A number [0.0, 1.0]
	F32 getDelta(F32 crntTime) const;

private:
	F32 startTime; ///< The time the event will start. Eg 23:00
	F32 duration; ///< The duration of the event
	EventManager* manager = nullptr; ///< Keep it here to access allocators etc
};

} // end namespace

#endif
