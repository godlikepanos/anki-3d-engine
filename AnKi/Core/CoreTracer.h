// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/CVarSet.h>

namespace anki {

/// @addtogroup core
/// @{

ANKI_CVAR(BoolCVar, Core, TracingEnabled, false, "Enable or disable tracing")
#if ANKI_OS_ANDROID
ANKI_CVAR(BoolCVar, Core, StreamlineAnnotations, false, "Enable or disable Streamline annotations")
#endif

#if ANKI_TRACING_ENABLED

/// A system that sits on top of the tracer and processes the counters and events.
class CoreTracer : public MakeSingleton<CoreTracer>
{
	template<typename>
	friend class MakeSingleton;

public:
	/// @param directory The directory to store the trace and counters.
	Error init(CString directory);

	/// It will flush everything.
	void flushFrame(U64 frame);

private:
	class ThreadWorkItem;
	class PerFrameCounters;

	Thread m_thread;
	ConditionVariable m_cvar;
	Mutex m_mtx;

	CoreDynamicArray<CoreString> m_counterNames;
	IntrusiveList<PerFrameCounters> m_frameCounters;

	IntrusiveList<ThreadWorkItem> m_workItems; ///< Items for the thread to process.
	CoreString m_traceJsonFilename;
	CoreString m_countersCsvFilename;
	File m_traceJsonFile;
	Bool m_quit = false;

	CoreTracer();

	~CoreTracer();

	Error threadWorker();

	Error writeEvents(ThreadWorkItem& item);
	void gatherCounters(ThreadWorkItem& item);
	Error writeCountersOnShutdown();
};

#endif
/// @}

} // end namespace anki
