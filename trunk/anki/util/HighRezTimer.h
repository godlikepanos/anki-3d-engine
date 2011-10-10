#ifndef HIGH_REZ_TIMER_H
#define HIGH_REZ_TIMER_H

#include "anki/util/StdTypes.h"


/// High resolution timer. All time in seconds
class HighRezTimer
{
	public:
		/// The type that the timer manipulates the results
		typedef double Scalar;

		HighRezTimer();

		/// Start the timer
		void start();

		/// Stop the timer
		void stop();

		/// Get the time elapsed between start and stop (if its stopped) or 
		/// between start and the current time
		Scalar getElapsedTime() const;

		/// Get the current date's seconds
		static Scalar getCrntTime();

	private:
		Scalar startTime;
		Scalar stopTime;
};


#endif
