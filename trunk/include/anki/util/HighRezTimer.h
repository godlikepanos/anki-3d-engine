#ifndef ANKI_UTIL_HIGH_REZ_TIMER_H
#define ANKI_UTIL_HIGH_REZ_TIMER_H

namespace anki {

/// High resolution timer. All time in seconds
class HighRezTimer
{
public:
	/// The type that the timer manipulates the results
	typedef double Scalar;

	/// Start the timer
	void start();

	/// Stop the timer
	void stop();

	/// Get the time elapsed between start and stop (if its stopped) or
	/// between start and the current time
	Scalar getElapsedTime() const;

	/// Get the current date's seconds
	static Scalar getCurrentTime();

private:
	Scalar startTime = 0.0;
	Scalar stopTime = 0.0;
};

} // end namespace

#endif
