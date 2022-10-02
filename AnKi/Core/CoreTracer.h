// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/File.h>

namespace anki {

/// @addtogroup core
/// @{

/// A system that sits on top of the tracer and processes the counters and events.
class CoreTracer
{
public:
	CoreTracer();

	~CoreTracer();

	/// @param directory The directory to store the trace and counters.
	Error init(HeapMemoryPool* pool, CString directory);

	/// It will flush everything.
	void flushFrame(U64 frame);

private:
	class ThreadWorkItem;
	class PerFrameCounters;

	HeapMemoryPool* m_pool = nullptr;

	Thread m_thread;
	ConditionVariable m_cvar;
	Mutex m_mtx;

	DynamicArray<String> m_counterNames;
	IntrusiveList<PerFrameCounters> m_frameCounters;

	IntrusiveList<ThreadWorkItem> m_workItems; ///< Items for the thread to process.
	File m_traceJsonFile;
	File m_countersCsvFile;
	Bool m_quit = false;

	Error threadWorker();

	Error writeEvents(ThreadWorkItem& item);
	void gatherCounters(ThreadWorkItem& item);
	Error writeCountersForReal();
};
/// @}

} // end namespace anki
