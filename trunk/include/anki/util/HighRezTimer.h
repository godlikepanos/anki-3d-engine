// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_HIGH_REZ_TIMER_H
#define ANKI_UTIL_HIGH_REZ_TIMER_H

#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util_time
/// @{

/// High resolution timer. All time in seconds
class HighRezTimer
{
public:
	/// The type that the timer manipulates the results
	typedef F64 Scalar;

	/// Start the timer
	void start();

	/// Stop the timer
	void stop();

	/// Get the time elapsed between start and stop (if its stopped) or
	/// between start and the current time
	Scalar getElapsedTime() const;

	/// Get the current date's seconds
	static Scalar getCurrentTime();

	/// Micro sleep
	static void sleep(Scalar seconds);

private:
	Scalar m_startTime = 0.0;
	Scalar m_stopTime = 0.0;
};
/// @}

} // end namespace anki

#endif
