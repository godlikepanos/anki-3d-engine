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
	RENDER,
	RENDER_MS,
	RENDER_IS,
	RENDER_SM,
	RENDER_DRAWER,
	GL_THREAD,
	SWAP_BUFFERS,

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

	void startEvent(TraceEventType type)
	{
		addEvent(type, true);
	}

	void stopEvent(TraceEventType type)
	{
		addEvent(type, false);
	}

	void flush();

private:
	class Entry
	{
	public:
		TraceEventType m_event;
		HighRezTimer::Scalar m_timestamp;
		Thread::Id m_tid;
		Bool8 m_start;
	};

	static const U BUFFERED_ENTRIES = 1024 * 10;
	Array<Entry, BUFFERED_ENTRIES> m_entries;
	Atomic<U32> m_count = {0};
	File m_file;

	void addEvent(TraceEventType type, Bool start)
	{
		U id = m_count.fetchAdd(1);
		if(id < BUFFERED_ENTRIES)
		{
			m_entries[id] = Entry{type, HighRezTimer::getCurrentTime(),
				Thread::getCurrentThreadId(), start};
		}
		else
		{
			ANKI_LOGW("Increase the buffered trace entries");
		}
	}
};

using TraceManagerSingleton = Singleton<TraceManager>;

/// @name Trace macros.
/// @{

#if ANKI_ENABLE_COUNTERS

#	define ANKI_TRACE_START_EVENT(name_) \
	TraceManagerSingleton::get().startEvent(TraceEventType::name_)

#	define ANKI_TRACE_STOP_EVENT(name_) \
	TraceManagerSingleton::get().stopEvent(TraceEventType::name_)

#	define ANKI_TRACE_FLUSH() \
	TraceManagerSingleton::get().flush()

#else

#	define ANKI_TRACE_START_EVENT(name_) ((void)0)
#	define ANKI_TRACE_STOP_EVENT(name_) ((void)0)
#	define ANKI_TRACE_FLUSH() ((void)0)

#endif
/// @}

} // end namespace anki

