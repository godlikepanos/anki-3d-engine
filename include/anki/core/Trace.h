// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Singleton.h>
#include <anki/util/Array.h>
#include <anki/util/HighRezTimer.h>
#include <anki/util/Thread.h>
#include <anki/util/Atomic.h>
#include <anki/util/Logger.h>
#include <anki/util/File.h>

namespace anki {

/// @addtogroup core
/// @{

/// Trace event type.
enum class TraceEventType
{
	SCENE_UPDATE,
	SCENE_DELETE_STUFF,
	SCENE_PHYSICS_UPDATE,
	SCENE_NODES_UPDATE,
	SCENE_VISIBILITY_TESTS,
	SCENE_VISIBILITY_TEST,
	SCENE_VISIBILITY_COMBINE_RESULTS,
	RENDER,
	RENDER_MS,
	RENDER_IS,
	RENDER_SM,
	RENDER_DRAWER,
	GL_THREAD,
	SWAP_BUFFERS,
	IDLE,

	COUNT
};

/// Trace counter type.
enum class TraceCounterType
{
	GR_DRAWCALLS,
	GR_DYNAMIC_UNIFORMS_SIZE,
	RENDERER_LIGHT_COUNT,

	COUNT
};

/// Trace manager.
class TraceManager
{
public:
	TraceManager()
	{}

	~TraceManager();

	ANKI_USE_RESULT Error create(HeapAllocator<U8> alloc,
		const CString& cacheDir);

	void startEvent();

	void stopEvent(TraceEventType type);

	void incCounter(TraceCounterType c, U64 val)
	{
		U idx = U(TraceEventType::COUNT) + U(c);
		m_perFrameCounters[idx].fetchAdd(val);
		m_perRunCounters[idx].fetchAdd(val);
	}

	void startFrame()
	{
		m_startFrameTime = HighRezTimer::getCurrentTime();
	}

	void stopFrame();

private:
	class Entry
	{
	public:
		TraceEventType m_event;
		HighRezTimer::Scalar m_timestamp; ///< When it started.
		HighRezTimer::Scalar m_duration;
		Thread::Id m_tid;
	};

	static const U BUFFERED_ENTRIES = 1024 * 10;
	Array<Entry, BUFFERED_ENTRIES> m_entries;
	Atomic<U32> m_count = {0};
	File m_file;
	HighRezTimer::Scalar m_startFrameTime;

	Array<Atomic<U64>, U(TraceEventType::COUNT) + U(TraceCounterType::COUNT)>
		m_perFrameCounters = {{}};
	Array<Atomic<U64>, U(TraceEventType::COUNT) + U(TraceCounterType::COUNT)>
		m_perRunCounters = {{}};

	void flushCounters();
};

using TraceManagerSingleton = Singleton<TraceManager>;

/// @name Trace macros.
/// @{

#if ANKI_ENABLE_COUNTERS

#	define ANKI_TRACE_START_EVENT(name_) \
	TraceManagerSingleton::get().startEvent()

#	define ANKI_TRACE_STOP_EVENT(name_) \
	TraceManagerSingleton::get().stopEvent(TraceEventType::name_)

#	define ANKI_TRACE_INC_COUNTER(name_, val_) \
	TraceManagerSingleton::get().incCounter(TraceCounterType:: name_, val_)

#	define ANKI_TRACE_START_FRAME() \
	TraceManagerSingleton::get().startFrame()

#	define ANKI_TRACE_STOP_FRAME() \
	TraceManagerSingleton::get().stopFrame()

#else

#	define ANKI_TRACE_START_EVENT(name_) ((void)0)
#	define ANKI_TRACE_STOP_EVENT(name_) ((void)0)
#	define ANKI_TRACE_START_FRAME() ((void)0)
#	define ANKI_TRACE_STOP_FRAME() ((void)0)

#endif
/// @}

} // end namespace anki

