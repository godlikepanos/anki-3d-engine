#include <boost/foreach.hpp>
#include "EventManager.h"


namespace Event {


//======================================================================================================================
// updateAllEvents                                                                                                     =
//======================================================================================================================
void Manager::updateAllEvents(uint prevUpdateTime, uint crntTime)
{
	BOOST_FOREACH(Event& event, events)
	{
		if(!event.isDead(crntTime))
		{
			event.update(prevUpdateTime, crntTime);
		}
	}
}


} // end namespace
