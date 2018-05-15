// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/Common.h>
#include <anki/util/Tracer.h>

namespace anki
{

/// @addtogroup core
/// @{

/// Core tracer.
class CoreTracer
{
public:
	Tracer m_tracer;
	Bool m_enabled = false;

	/// @copydoc Tracer::init
	void init(GenericMemoryPoolAllocator<U8> alloc)
	{
		m_tracer.init(alloc);
	}

	/// @copydoc Tracer::isInitialized
	Bool isInitialized() const
	{
		return m_tracer.isInitialized();
	}

	/// @copydoc Tracer::beginEvent
	ANKI_USE_RESULT TracerEventHandle beginEvent()
	{
		if(m_enabled)
		{
			return m_tracer.beginEvent();
		}

		return nullptr;
	}

	/// @copydoc Tracer::endEvent
	void endEvent(const char* eventName, TracerEventHandle event)
	{
		if(event != nullptr)
		{
			m_tracer.endEvent(eventName, event);
		}
	}

	/// @copydoc Tracer::increaseCounter
	void increaseCounter(const char* counterName, U64 value)
	{
		if(m_enabled)
		{
			m_tracer.increaseCounter(counterName, value);
		}
	}

	/// @copydoc Tracer::newFrame
	void newFrame(U64 frame)
	{
		if(m_enabled)
		{
			m_tracer.newFrame(frame);
		}
	}

	/// @copydoc Tracer::flush
	ANKI_USE_RESULT Error flush(CString filename)
	{
		return m_tracer.flush(filename);
	}
};

using CoreTracerSingleton = Singleton<CoreTracer>;

class CoreTraceScopedEvent
{
public:
	CoreTraceScopedEvent(const char* name)
		: m_name(name)
		, m_tracer(&CoreTracerSingleton::get())
	{
		m_handle = m_tracer->beginEvent();
	}

	~CoreTraceScopedEvent()
	{
		m_tracer->endEvent(m_name, m_handle);
	}

private:
	const char* m_name;
	TracerEventHandle m_handle;
	CoreTracer* m_tracer;
};

/// @name Trace macros.
/// @{
#if ANKI_ENABLE_TRACE
#	define ANKI_TRACE_START_EVENT(name_) TracerEventHandle _teh##name_ = CoreTracerSingleton::get().beginEvent()
#	define ANKI_TRACE_STOP_EVENT(name_) CoreTracerSingleton::get().endEvent(#	name_, _teh##name_)
#	define ANKI_TRACE_SCOPED_EVENT(name_) CoreTraceScopedEvent _tse##name_(#	name_)
#	define ANKI_TRACE_INC_COUNTER(name_, val_) CoreTracerSingleton::get().increaseCounter(#	name_, val_)
#else
#	define ANKI_TRACE_START_EVENT(name_) ((void)0)
#	define ANKI_TRACE_STOP_EVENT(name_) ((void)0)
#	define ANKI_TRACE_SCOPED_EVENT(name_) ((void)0)
#	define ANKI_TRACE_INC_COUNTER(name_, val_) ((void)0)
#endif
/// @}

} // end namespace anki