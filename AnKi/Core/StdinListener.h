// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/List.h>

namespace anki {

/// @addtogroup core
/// @{

/// The listener of the stdin. It initiates a thread that constantly reads the stdin and puts the results in a queue
class StdinListener
{
public:
	StdinListener()
		: m_thrd("Stdin")
	{
	}

	~StdinListener();

	Error create(HeapMemoryPool* pool);

	/// Get line from the queue or return an empty string
	String getLine();

private:
	HeapMemoryPool* m_pool = nullptr;
	List<String> m_q;
	Mutex m_mtx; ///< Protect the queue
	Thread m_thrd; ///< The thread
	Bool m_quit = false;

	static Error workingFunc(ThreadCallbackInfo& info); ///< The thread function
};

/// @}

} // end namespace anki
