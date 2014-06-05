// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlSync.h"
#include "anki/gl/GlJobChainHandle.h"
#include "anki/gl/GlManager.h"
#include "anki/core/Counters.h"


namespace anki {

//==============================================================================
/// Wait job
class GlClientSyncWaitJob: public GlJob
{
public:
	GlClientSyncHandle m_sync;	

	GlClientSyncWaitJob(const GlClientSyncHandle& s)
		: m_sync(s)
	{}

	void operator()(GlJobChain*)
	{
		ANKI_COUNTER_START_TIMER(GL_SERVER_WAIT_TIME);
		m_sync._get().wait();
		ANKI_COUNTER_STOP_TIMER_INC(GL_SERVER_WAIT_TIME);
	}
};

//==============================================================================
GlClientSyncHandle::GlClientSyncHandle()
{}

//==============================================================================
GlClientSyncHandle::GlClientSyncHandle(GlJobChainHandle& jobs)
{
	auto alloc = jobs._getGlobalAllocator();

	typedef GlHandleDefaultDeleter<GlClientSync, GlGlobalHeapAllocator<U8>>
		Deleter;

	*static_cast<Base*>(this) = Base(
		&jobs._getJobManager().getManager(), alloc, Deleter());
}

//==============================================================================
GlClientSyncHandle::~GlClientSyncHandle()
{}

//==============================================================================
void GlClientSyncHandle::sync(GlJobChainHandle& jobs)
{
	jobs._pushBackNewJob<GlClientSyncWaitJob>(*this);
}

//==============================================================================
void GlClientSyncHandle::wait()
{
	ANKI_COUNTER_START_TIMER(GL_CLIENT_WAIT_TIME);
	_get().wait();
	ANKI_COUNTER_STOP_TIMER_INC(GL_CLIENT_WAIT_TIME);
}

} // end namespace anki

