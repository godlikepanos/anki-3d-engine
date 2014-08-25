// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_STDIN_LISTENER_H
#define ANKI_CORE_STDIN_LISTENER_H

#include "anki/util/String.h"
#include "anki/util/Thread.h"
#include <queue>

namespace anki {

/// @addtogroup core
/// @{

/// The listener of the stdin.
/// It initiates a thread that constantly reads the stdin and puts the results
/// in a queue
class StdinListener
{
public:
	StdinListener();

	~StdinListener();

	/// Get line from the queue or return an empty string
	String getLine();

private:
	std::queue<String> m_q;
	Mutex m_mtx; ///< Protect the queue
	Thread m_thrd; ///< The thread
	Bool8 m_quit = false;

	static I workingFunc(Thread::Info& info); ///< The thread function
};

/// @}

} // end namespace anki

#endif
