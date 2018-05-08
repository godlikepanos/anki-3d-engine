// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/Common.h>
#include <anki/util/Tracer.h>

/// @name Trace macros.
/// @{
#if ANKI_ENABLE_TRACE
#	define ANKI_TRACE_START_EVENT(name_) TracerEventHandle _teh##name_ = TracerSingleton::get().beginEvent()
#	define ANKI_TRACE_STOP_EVENT(name_) TracerSingleton::get().endEvent(#	name_, _teh##name_)
#	define ANKI_TRACE_SCOPED_EVENT(name_) TraceScopedEvent _tse##name_(#	name_)
#	define ANKI_TRACE_INC_COUNTER(name_, val_) TracerSingleton::get().increaseCounter(#	name_, val_)
#else
#	define ANKI_TRACE_START_EVENT(name_) ((void)0)
#	define ANKI_TRACE_STOP_EVENT(name_) ((void)0)
#	define ANKI_TRACE_SCOPED_EVENT(name_) ((void)0)
#	define ANKI_TRACE_INC_COUNTER(name_, val_) ((void)0)
#endif
/// @}
