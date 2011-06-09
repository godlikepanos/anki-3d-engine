#include "Event.h"
#include "Assert.h"


namespace Event {


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Event::Event(EventType type_, uint startTime_, int duration_):
	type(type_),
	startTime(startTime_),
	duration(duration_)
{}


//======================================================================================================================
// operator=                                                                                                           =
//======================================================================================================================
Event& Event::operator=(const Event& b)
{
	type = b.type;
	startTime = b.startTime;
	duration = b.duration;
	return *this;
}


//======================================================================================================================
// update                                                                                                              =
//======================================================================================================================
void Event::update(uint prevUpdateTime, uint crntTime)
{
	ASSERT(!isDead(crntTime));

	// Dont update if its not the right time yet
	if(startTime >= crntTime)
	{
		updateSp(prevUpdateTime, crntTime);
	}
}


} // end namespace
