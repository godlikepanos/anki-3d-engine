// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>

namespace anki
{

/// @addtogroup util_time
/// @{

/// High resolution timer. All time in seconds
class HighRezTimer
{
public:
	/// The type that the timer manipulates the results
	using Scalar = F64;

	/// Start the timer
	void start();

	/// Stop the timer
	void stop();

	/// Get the time elapsed between start and stop (if its stopped) or between start and the current time.
	Scalar getElapsedTime() const;

	/// Get the current date's seconds
	static Scalar getCurrentTime();

	/// Micro sleep. The resolution is in nanoseconds.
	static void sleep(Scalar seconds);

private:
	Scalar m_startTime = 0.0;
	Scalar m_stopTime = 0.0;
};
/// @}

} // end namespace anki
