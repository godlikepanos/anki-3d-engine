#ifndef ANKI_EVENT_EVENT_H
#define ANKI_EVENT_EVENT_H

#include "anki/scene/Common.h"
#include "anki/util/Bitset.h"
#include "anki/Math.h"

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
	/// @param prevUpdateTime The time of the previous update (sec)
	/// @param crntTime The current time (sec)
	virtual void update(F32 prevUpdateTime, F32 crntTime) = 0;

	/// This is called when the event is killed
	/// @param prevUpdateTime The time of the previous update (sec)
	/// @param crntTime The current time (sec)
	/// @return Return false if you don't want to be killed
	virtual Bool onKilled(F32 prevUpdateTime, F32 crntTime)
	{
		return true;
	}

protected:
	/// Linear interpolation between values
	/// @param[in] from Starting value
	/// @param[in] to Ending value
	/// @param[in] u The percentage from the from "from" value. Values
	///              from [0.0, 1.0]
	template<typename Type>
	static Type interpolate(const Type& from, const Type& to, F32 u)
	{
		//ANKI_ASSERT(u >= 0 && u <= 1.0);
		return from * (1.0 - u) + to * u;
	}

	template<typename Type>
	static Type cosInterpolate(const Type& from, const Type& to, F32 u)
	{
		F32 u2 = (1.0 - cos(u * getPi<F32>())) / 2.0;
		return from * (1.0 - u2) + to * u2;
	}

	template<typename Type>
	static Type cubicInterpolate(
		const Type& a, const Type& b, const Type& c, 
		const Type& d, F32 u)
	{
		F32 u2 = u * u;
		Type a0 = d - c - a + b;
		Type a1 = a - b - a0;
		Type a2 = c - a;
		Type a3 = b;

		return(a0 * u * u2 + a1 * u2 + a2 * u + a3);
	}

	/// Return the u between current time and when the event started
	/// @return A number [0.0, 1.0]
	F32 getDelta(F32 crntTime) const;

private:
	/// The time the event will start. Eg 23:00. If it's < 0 then start the 
	/// event now
	F32 startTime;
	F32 duration; ///< The duration of the event
	EventManager* manager = nullptr; ///< Keep it here to access allocators etc
};

} // end namespace anki

#endif
