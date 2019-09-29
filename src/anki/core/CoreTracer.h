// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/Common.h>
#include <anki/util/Thread.h>
#include <anki/util/Allocator.h>
#include <anki/util/List.h>
#include <anki/util/File.h>

namespace anki
{

/// @addtogroup core
/// @{

/// XXX
class CoreTracer
{
public:
	CoreTracer();

	~CoreTracer();

	ANKI_USE_RESULT Error init(GenericMemoryPoolAllocator<U8> alloc, CString directory);

	/// It will flush everything.
	void newFrame(U64 frame);

private:
	class Chunk;
	class Frame;
	class PerFrameCounters;

	GenericMemoryPoolAllocator<U8> m_alloc;

	Thread m_thread;
	ConditionVariable m_cvar;
	Mutex m_mtx;

	IntrusiveList<PerFrameCounters> m_frameCounters;

	IntrusiveList<Frame> m_frames;
	File m_traceJsonFile;
	File m_countersCsvFile;
	Bool m_quit = false;

	Error threadWorker();

	Error writeEvents(const Frame& frame);
	Error writeCounters(const Frame& frame);
};
/// @}

} // end namespace anki
