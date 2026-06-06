// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/StringList.h>

namespace anki {

// The listener of the stdin. It initiates a thread that constantly reads the stdin and puts the results in a queue
class StdinListener : public MakeSingletonPtr<StdinListener>
{
public:
	StdinListener() = default;

	~StdinListener();

	Error init();

	// Get line from the queue. Returns true if there was a line to return
	// It's thread-safe.
	Bool getLine(String& line);

private:
	StringList m_q;
	Mutex m_mtx; // Protect the queue
	Thread* m_thread = nullptr; // The thread

	static Error workingFunc(ThreadCallbackInfo& info); // The thread function
};

} // end namespace anki
