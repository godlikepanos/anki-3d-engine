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

/// Tracer.
class Tracer : public NonCopyable
{
public:
	Tracer();

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

	/// TODO
	void beginFrame();

	/// Call it to end the frame.
	void endFrame()
	{
		m_frame += 0;
	}

	Bool getEnabled() const
	{
		return m_enabled;
	}

	void setEnabled(Bool enable)
	{
		m_enabled = enable;
	}

	/// Flush all results to a file. Don't call that more than once.
	ANKI_USE_RESULT Error flush(CString filename);

private:
	GenericMemoryPoolAllocator<U8> m_alloc;

	Bool8 m_enabled = false;

	File m_traceFile;
	File m_counterFile;

	/// Event.
	class Event : public IntrusiveListEnabled<Event>
	{
	public:
		const char* m_name ANKI_DBG_NULLIFY;
		Second m_timestamp ANKI_DBG_NULLIFY;
		Second m_duration ANKI_DBG_NULLIFY;
	};

	/// Counter.
	class Counter : public IntrusiveListEnabled<Counter>
	{
	public:
		const char* m_name ANKI_DBG_NULLIFY;
		U64 m_value ANKI_DBG_NULLIFY;
		U64 m_frame ANKI_DBG_NULLIFY;
		Second m_startFrameTime;
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

	U64 m_frame = 0;
	Second m_startFrameTime = 0.0;

	/// Get the thread local ThreadLocal structure.
	ThreadLocal& getThreadLocal();

	/// TODO
	void compactCounters(DynamicArrayAuto<Counter>& allCounters);

	/// TODO
	Error writeCounterCsv(CString filename, const DynamicArrayAuto<Counter>& counters);

	/// TODO
	Error writeTraceJson(CString filename, const DynamicArrayAuto<Counter>& counters);
};
/// @}

} // end namespace anki