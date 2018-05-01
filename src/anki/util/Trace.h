// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/File.h>

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

	ANKI_USE_RESULT Error init(GenericMemoryPoolAllocator<U8> alloc, const CString& cacheDir);

	void registerEvent(const char* name);

	void registerCounter(const char* name);

	void startEvent();

	void stopEvent(U64 hash);

	void increaseCounter(const char* eventName, U64 hash, U64 value);

	/// Call it to begin the frame.
	void beginFrame();

	/// Call it to end the frame.
	void endFrame();

	Bool getEnabled() const
	{
		return m_enabled;
	}

	void setEnabled(Bool enable)
	{
		m_enabled = enable;
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;

	Bool8 m_enabled = false;

	File m_traceFile;
	File m_counterFile;

	struct Event
	{
		const char* m_name;
		Second m_timestamp;
		Second m_duration;
		ThreadId m_tid;
	};

	struct Counter
	{
		const char* m_name;
		U64 m_hash;
		U64 m_value;
	};

	class PerThread
	{
	public:
		ThreadId m_tid;
		DynamicArray<Event> m_events;
		DynamicArray<Counter> m_counters;
	};

	static thread_local PerThread* m_perThread;

	DynamicArray<PerThread> m_perThread;
	Mutex m_perThreadMtx;
};
/// @}

} // end namespace anki