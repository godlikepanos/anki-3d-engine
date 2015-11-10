// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/Trace.h>

namespace anki {

//==============================================================================
static Array<const char*, U(TraceEventType::COUNT)> traceNames = {{
	"SCENE_UPDATE",
	"SCENE_DELETE_STUFF",
	"SCENE_PHYSICS_UPDATE",
	"SCENE_NODES_UPDATE",
	"SCENE_VISIBILITY_TESTS",
	"RENDER",
	"RENDER_MS",
	"RENDER_IS",
	"RENDER_SM",
	"RENDER_DRAWER",
	"GL_THREAD",
	"SWAP_BUFFERS"
}};

//==============================================================================
TraceManager::~TraceManager()
{
	if(m_file.isOpen())
	{
		Error err = m_file.writeText(R"({"name": "dummy", "cat": "PERF", )"
			R"("ph": "E", "pid": 666, "tid": 0, "ts": 0}]})");

		if(err)
		{
			ANKI_LOGE("Error writting the trace file");
		}
	}
}

//==============================================================================
Error TraceManager::create(HeapAllocator<U8> alloc, const CString& cacheDir)
{
	// Open and write some stuff to the file
	StringAuto fname(alloc);
	fname.sprintf("%s/trace.json", &cacheDir[0]);

	ANKI_CHECK(m_file.open(fname.toCString(), File::OpenFlag::WRITE));
	ANKI_CHECK(m_file.writeText(
		"{\n"
		"\"displayTimeUnit\": \"ms\",\n"
		"\"traceEvents\": [\n"));

	return ErrorCode::NONE;
}

//==============================================================================
void TraceManager::flush()
{
	U count = m_count.exchange(0);
	count = min<U>(count, BUFFERED_ENTRIES);

	for(U i = 0; i < count; ++i)
	{
		const Entry& e = m_entries[i];

		Error err = m_file.writeText(
			"{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"%s\", "
			"\"pid\": 666, \"tid\": %llu, \"ts\": %llu},\n",
			traceNames[e.m_event], (e.m_start ? "B" : "E"), e.m_tid,
			U64(e.m_timestamp * 1000000.0));

		if(err)
		{
			ANKI_LOGE("Error writting the trace file");
		}
	}
}

} // end namespace anki
