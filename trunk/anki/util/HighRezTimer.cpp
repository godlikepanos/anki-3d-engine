#include "anki/util/HighRezTimer.h"
#include "anki/util/Assert.h"
#include <boost/date_time/posix_time/posix_time.hpp>


namespace anki {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
HighRezTimer::HighRezTimer():
	startTime(0.0),
	stopTime(0.0)
{}


//==============================================================================
// start                                                                       =
//==============================================================================
void HighRezTimer::start()
{
	ASSERT(startTime == 0);
	ASSERT(stopTime == 0);
	startTime = getCrntTime();
	stopTime = 0.0;
}


//==============================================================================
// stop                                                                        =
//==============================================================================
void HighRezTimer::stop()
{
	ASSERT(startTime != 0.0);
	ASSERT(stopTime == 0.0);
	stopTime = getCrntTime();
}


//==============================================================================
// getElapsedTime                                                              =
//==============================================================================
HighRezTimer::Scalar HighRezTimer::getElapsedTime() const
{
	if(stopTime == 0)
	{
		return getCrntTime() - startTime;
	}
	else
	{
		return stopTime - startTime;
	}
}


//==============================================================================
// getCrntTime                                                                 =
//==============================================================================
HighRezTimer::Scalar HighRezTimer::getCrntTime()
{
	using namespace boost::posix_time;
	ulong ms = ptime(microsec_clock::local_time()).time_of_day().
		total_milliseconds();
	return Scalar(ms) / 1000.0;
}


} // end namespace
