#include "anki/util/HighRezTimer.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
void HighRezTimer::start()
{
	ANKI_ASSERT(startTime == 0);
	ANKI_ASSERT(stopTime == 0);
	startTime = getCurrentTime();
	stopTime = 0.0;
}

//==============================================================================
void HighRezTimer::stop()
{
	ANKI_ASSERT(startTime != 0.0);
	ANKI_ASSERT(stopTime == 0.0);
	stopTime = getCurrentTime();
}

//==============================================================================
HighRezTimer::Scalar HighRezTimer::getElapsedTime() const
{
	if(stopTime == 0.0)
	{
		return getCurrentTime() - startTime;
	}
	else
	{
		return stopTime - startTime;
	}
}

} // end namespace anki
