// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/Common.h>
#include <anki/util/String.h>
#include <anki/util/Thread.h>
#include <anki/util/List.h>

namespace anki
{

/// @addtogroup core
/// @{

/// The listener of the stdin. It initiates a thread that constantly reads the stdin and puts the results in a queue
class StdinListener
{
public:
	StdinListener()
		: m_thrd("anki_stdin")
	{
	}

	~StdinListener();

	ANKI_USE_RESULT Error create(HeapAllocator<String>& alloc);

	/// Get line from the queue or return an empty string
	String getLine();

private:
	HeapAllocator<U8> m_alloc;
	List<String> m_q;
	Mutex m_mtx; ///< Protect the queue
	Thread m_thrd; ///< The thread
	Bool m_quit = false;

	static Error workingFunc(ThreadCallbackInfo& info); ///< The thread function
};

/// @}

} // end namespace anki
