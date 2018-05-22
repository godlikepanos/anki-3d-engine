// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/File.h>
#include <anki/util/List.h>
#include <anki/util/ObjectAllocator.h>

namespace anki
{

/// @addtogroup util_other
/// @{

/// @memberof Tracer
using TracerEventHandle = void*;

/// Tracer.
class Tracer : public NonCopyable
{
public:
	Tracer()
	{
	}

	~Tracer();

	void init(GenericMemoryPoolAllocator<U8> alloc)
	{
		m_alloc = alloc;
	}

	Bool isInitialized() const
	{
		return !!m_alloc;
	}

	/// Begin a new event.
	ANKI_USE_RESULT TracerEventHandle beginEvent();

	/// End the event that got started with beginEvent().
	void endEvent(const char* eventName, TracerEventHandle event);

	/// Increase a counter.
	void increaseCounter(const char* counterName, U64 value);

	/// Begin a new frame.
	void newFrame(U64 frame);

	/// Flush all results to a file. Don't call that more than once.
	ANKI_USE_RESULT Error flush(CString filename);

private:
	static const U32 EVENTS_PER_CHUNK = 256;
	static const U32 COUNTERS_PER_CHUNK = 512;

	class Event;
	class EventsChunk;
	class GatherEvent;

	class Counter;
	class CountersChunk;
	class GatherCounter;

	class ThreadLocal;
	class PerFrameCounters;
	class FlushCtx;

	GenericMemoryPoolAllocator<U8> m_alloc;

	Second m_startFrameTime = 0.0;
	U64 m_frame = 0;
	SpinLock m_frameMtx; ///< Protect m_startFrameTime and m_frame.

	static thread_local ThreadLocal* m_threadLocal;
	DynamicArray<ThreadLocal*> m_allThreadLocal; ///< The Tracer should know about all the ThreadLocal.
	Mutex m_threadLocalMtx;

	/// Get the thread local ThreadLocal structure.
	ThreadLocal& getThreadLocal();

	/// Gather all counters from all the threads.
	void gatherCounters(FlushCtx& ctx);

	/// Gather the events from all the threads.
	void gatherEvents(FlushCtx& ctx);

	/// Dump the counters to a CSV file
	Error writeCounterCsv(const FlushCtx& ctx);

	/// Dump the events and the counters to a chrome trace file.
	Error writeTraceJson(const FlushCtx& ctx);

	static void getSpreadsheetColumnName(U column, Array<char, 3>& arr);
};
/// @}

} // end namespace anki