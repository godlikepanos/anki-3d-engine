// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Thread.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/DynamicArray.h>
#include <AnKi/Util/Singleton.h>
#include <AnKi/Util/String.h>

namespace anki {

/// @addtogroup util_other
/// @{

/// @memberof Tracer
class TracerEventHandle
{
	friend class Tracer;

private:
	Second m_start;
};

/// @memberof Tracer
class TracerEvent
{
public:
	CString m_name;
	Second m_start;
	Second m_duration;

	TracerEvent()
	{
		// No init
	}
};

/// @memberof Tracer
class TracerCounter
{
public:
	CString m_name;
	U64 m_value;

	TracerCounter()
	{
		// No init
	}
};

/// Tracer flush callback.
/// @memberof Tracer
using TracerFlushCallback = void (*)(void* userData, ThreadId tid, ConstWeakArray<TracerEvent> events,
									 ConstWeakArray<TracerCounter> counters);

/// Tracer.
class Tracer : public MakeSingleton<Tracer>
{
	template<typename>
	friend class MakeSingleton;

public:
	Tracer(const Tracer&) = delete; // Non-copyable

	Tracer& operator=(const Tracer&) = delete; // Non-copyable

	/// Begin a new event.
	/// @note It's thread-safe.
	[[nodiscard]] TracerEventHandle beginEvent();

	/// End the event that got started with beginEvent().
	/// @note It's thread-safe.
	void endEvent(const char* eventName, TracerEventHandle event);

	/// Add a custom event.
	/// @note It's thread-safe.
	void addCustomEvent(const char* eventName, Second start, Second duration);

	/// Increment a counter.
	/// @note It's thread-safe.
	void incrementCounter(const char* counterName, U64 value);

	/// Flush all counters and events and start clean. The callback will be called multiple times.
	/// @note It's thread-safe.
	void flush(TracerFlushCallback callback, void* callbackUserData);

	Bool getEnabled() const
	{
		return m_enabled;
	}

	void setEnabled(Bool enabled)
	{
		m_enabled = enabled;
	}

private:
	static constexpr U32 kEventsPerChunk = 256;
	static constexpr U32 kCountersPerChunk = 512;

	class ThreadLocal;
	class Chunk;

	static thread_local ThreadLocal* m_threadLocal;

	/// The Tracer should know about all the ThreadLocal.
	DynamicArray<ThreadLocal*, SingletonMemoryPoolWrapper<DefaultMemoryPool>> m_allThreadLocal;

	Mutex m_allThreadLocalMtx;

	Bool m_enabled = false;

	Tracer() = default;

	~Tracer();

	/// Get the thread local ThreadLocal structure.
	/// @note Thread-safe.
	ThreadLocal& getThreadLocal();

	/// Get or create a new chunk.
	Chunk& getOrCreateChunk(ThreadLocal& tlocal);
};

/// Scoped tracer event.
class TracerScopedEvent
{
public:
	TracerScopedEvent(const char* name)
		: m_name(name)
		, m_tracer(&Tracer::getSingleton())
	{
		m_handle = m_tracer->beginEvent();
	}

	~TracerScopedEvent()
	{
		m_tracer->endEvent(m_name, m_handle);
	}

private:
	const char* m_name;
	TracerEventHandle m_handle;
	Tracer* m_tracer;
};

#if ANKI_ENABLE_TRACE
#	define ANKI_TRACE_SCOPED_EVENT(name_) TracerScopedEvent _tse##name_(ANKI_STRINGIZE(ANKI_CONCATENATE(t, name_)))
#	define ANKI_TRACE_CUSTOM_EVENT(name_, start_, duration_) \
		Tracer::getSingleton().addCustomEvent(ANKI_STRINGIZE(ANKI_CONCATENATE(t, name_)), start_, duration_)
#	define ANKI_TRACE_INC_COUNTER(name_, val_) \
		Tracer::getSingleton().incrementCounter(ANKI_STRINGIZE(ANKI_CONCATENATE(c, name_)), val_)
#else
#	define ANKI_TRACE_SCOPED_EVENT(name_) ((void)0)
#	define ANKI_TRACE_CUSTOM_EVENT(name_, start_, duration_) ((void)0)
#	define ANKI_TRACE_INC_COUNTER(name_, val_) ((void)0)
#endif
/// @}

} // end namespace anki
