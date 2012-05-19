#include "anki/util/HighRezTimer.h"
#include "anki/util/Assert.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace anki {

//==============================================================================
void HighRezTimer::start()
{
	ANKI_ASSERT(startTime == 0);
	ANKI_ASSERT(stopTime == 0);
	startTime = getCrntTime();
	stopTime = 0.0;
}

//==============================================================================
void HighRezTimer::stop()
{
	ANKI_ASSERT(startTime != 0.0);
	ANKI_ASSERT(stopTime == 0.0);
	stopTime = getCrntTime();
}

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
HighRezTimer::Scalar HighRezTimer::getCrntTime()
{
	/// XXX Remove the boost
	using namespace boost::posix_time;
	uint64_t ms = ptime(microsec_clock::local_time()).time_of_day().
		total_milliseconds();
	return Scalar(ms) / 1000.0;
}

} // end namespace
