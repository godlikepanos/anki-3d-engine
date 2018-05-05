// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/File.h>
#include <anki/util/List.h>
#include <anki/util/ObjectAllocator.h>
#include <anki/util/Singleton.h>

namespace anki
{

/// @addtogroup util_other
/// @{

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

	/// Begin a new event.
	void beginEvent();

	/// End the event that got started with beginEvent().
	void endEvent(const char* eventName);

	/// Increase a counter.
	void increaseCounter(const char* counterName, U64 value);

	/// Begin a new frame.
	void beginFrame(U64 frame);

	/// Call it to end the frame.
	void endFrame();

	/// Flush all results to a file. Don't call that more than once.
	ANKI_USE_RESULT Error flush(CString filename);

private:
	GenericMemoryPoolAllocator<U8> m_alloc;

	class Frame
	{
	public:
		U64 m_frame;
		Second m_startFrameTime; ///< When the frame started
		Second m_endFrameTime; ///< When it ended
#if ANKI_ASSERTS_ENABLED
		Bool8 m_canRecord;
#endif
	};

	DynamicArray<Frame> m_frames;

	/// Event.
	class Event : public IntrusiveListEnabled<Event>
	{
	public:
		const char* m_name ANKI_DBG_NULLIFY;
		Second m_timestamp ANKI_DBG_NULLIFY;
		Second m_duration ANKI_DBG_NULLIFY;
		ThreadId m_tid ANKI_DBG_NULLIFY;
	};

	/// Counter.
	class Counter : public IntrusiveListEnabled<Counter>
	{
	public:
		const char* m_name ANKI_DBG_NULLIFY;
		U64 m_value ANKI_DBG_NULLIFY;
		U32 m_frameIdx ANKI_DBG_NULLIFY;
	};

	class ThreadLocal
	{
	public:
		ThreadId m_tid ANKI_DBG_NULLIFY;
		ObjectAllocatorSameType<Event> m_eventAlloc;
		ObjectAllocatorSameType<Counter> m_counterAlloc;
		IntrusiveList<Event> m_events;
		IntrusiveList<Counter> m_counters;
		Bool m_tracerKnowsAboutThis = false;
	};

	static thread_local ThreadLocal m_threadLocal;
	DynamicArray<ThreadLocal*> m_allThreadLocal; ///< The Tracer should know about all the ThreadLocal.
	Mutex m_threadLocalMtx;

	class FlushCtx;
	class PerFrameCounters;

	Bool isInsideBeginEndFrame() const
	{
		return m_frames.getSize() > 0 && m_frames.getBack().m_canRecord;
	}

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
};

/// Tracer singleton.
using TracerSingleton = Singleton<Tracer>;

/// Convenience class to trace an event.
class TraceScopedEvent
{
public:
	TraceScopedEvent(const char* name)
		: m_name(name)
		, m_tracer(&TracerSingleton::get())
	{
		m_tracer->beginEvent();
	}

	~TraceScopedEvent()
	{
		m_tracer->endEvent(m_name);
	}

private:
	const char* m_name;
	Tracer* m_tracer;
};

/// Convenience class to increase a trace counter.
class TraceIncreaseCounter
{
public:
	TraceIncreaseCounter(const char* name, U64 value)
	{
		TracerSingleton::get().increaseCounter(name, value);
	}
};
/// @}

} // end namespace anki