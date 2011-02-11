#ifndef HIGH_REZ_TIMER_H
#define HIGH_REZ_TIMER_H

#include "StdTypes.h"


/// High resoluton timer. All time in milliseconds
class HighRezTimer
{
	public:
		HighRezTimer();

		/// Start the timer
		void start();

		/// Stop the timer
		void stop();

		/// Get the time elapsed between start and stop (if its stopped) or between start and the current time
		uint getElapsedTime() const;

		/// Get the current date's millisecond
		static uint getCrntTime();

	private:
		uint startTime;
		uint stopTime;
};


#endif
