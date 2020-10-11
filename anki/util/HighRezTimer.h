// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
	/// Start the timer
	void start();

	/// Stop the timer
	void stop();

	/// Get the time elapsed between start and stop (if its stopped) or between start and the current time.
	Second getElapsedTime() const;

	/// Get the current date's seconds
	static Second getCurrentTime();

	/// Micro sleep. The resolution is in nanoseconds.
	static void sleep(Second seconds);

private:
	Second m_startTime = 0.0;
	Second m_stopTime = 0.0;
};
/// @}

} // end namespace anki
