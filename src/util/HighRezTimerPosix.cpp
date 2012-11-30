#include "anki/util/HighRezTimer.h"
#include "anki/util/Assert.h"
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#if !defined(HAVE_NANOSLEEP)
#	define HAVE_NANOSLEEP 0
#endif

#if !defined(HAVE_CLOCK_GETTIME)
#	define HAVE_CLOCK_GETTIME 0
#endif

#if HAVE_NANOSLEEP || HAVE_CLOCK_GETTIME
#	include <time.h>
#endif

namespace anki {

//==============================================================================
// The first ticks value of the application
#if HAVE_CLOCK_GETTIME
static struct timespec gstart;
#else
static struct timeval gstart;
#endif

/// A dummy struct that inits the timer
struct DummyInitTimer
{
	DummyInitTimer()
	{
#if HAVE_CLOCK_GETTIME
		clock_gettime(CLOCK_MONOTONIC, &gstart);
#else
		gettimeofday(&gstart, NULL);
#endif
	}
};

static DummyInitTimer dummy;

//==============================================================================
static U32 getMs()
{
	U32 ticks;

#if HAVE_CLOCK_GETTIME
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	ticks = (now.tv_sec - gstart.tv_sec) * 1000 
		+ (now.tv_nsec - gstart.tv_nsec) / 1000000;
#else
	struct timeval now;
	gettimeofday(&now, NULL);
	ticks = (now.tv_sec - gstart.tv_sec) * 1000 
		+ (now.tv_usec - gstart.tv_usec) / 1000;
#endif

	return ticks;
}

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

//==============================================================================
HighRezTimer::Scalar HighRezTimer::getCurrentTime()
{
	// Scalar(ticks) / 1000.0
	return Scalar(getMs()) * 0.001;
}

//==============================================================================
void HighRezTimer::sleep(Scalar sec)
{
	int wasError;
	U32 ms = U32(sec * 1000.0);

#if HAVE_NANOSLEEP
	struct timespec elapsed, tv;
#else
	struct timeval tv;
	U32 then, now, elapsed;
#endif

	// Set the timeout interval
#if HAVE_NANOSLEEP
	elapsed.tv_sec = ms / 1000;
	elapsed.tv_nsec = (ms % 1000) * 1000000;
#else
	then = getMs();
#endif

	do 
	{
		errno = 0;

#if HAVE_NANOSLEEP
		tv.tv_sec = elapsed.tv_sec;
		tv.tv_nsec = elapsed.tv_nsec;
		wasError = nanosleep(&tv, &elapsed);
#else
		// Calculate the time interval left (in case of interrupt)
		now = getMs();
		elapsed = now - then;
		then = now;
		if(elapsed >= ms)
		{
			break;
		}
		ms -= elapsed;
		tv.tv_sec = ms / 1000;
		tv.tv_usec = (ms % 1000) * 1000;

		wasError = select(0, NULL, NULL, NULL, &tv);
#endif
	} while(wasError && (errno == EINTR));
}

} // end namespace anki
