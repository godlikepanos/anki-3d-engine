// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlSync.h"
#include "anki/gl/GlCommandBufferHandle.h"
#include "anki/gl/GlDevice.h"
#include "anki/core/Counters.h"


namespace anki {

//==============================================================================
/// Wait command
class GlClientSyncWaitCommand: public GlCommand
{
public:
	GlClientSyncHandle m_sync;	

	GlClientSyncWaitCommand(const GlClientSyncHandle& s)
	:	m_sync(s)
	{}

	void operator()(GlCommandBuffer*)
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
GlClientSyncHandle::~GlClientSyncHandle()
{}

//==============================================================================
Error GlClientSyncHandle::create(GlCommandBufferHandle& commands)
{
	auto alloc = commands._getGlobalAllocator();

	using Deleter = 
		GlHandleDefaultDeleter<GlClientSync, GlAllocator<U8>>;

	return _createAdvanced(
		&commands._getQueue().getDevice(), alloc, Deleter());
}

//==============================================================================
void GlClientSyncHandle::sync(GlCommandBufferHandle& commands)
{
	commands._pushBackNewCommand<GlClientSyncWaitCommand>(*this);
}

//==============================================================================
void GlClientSyncHandle::wait()
{
	ANKI_COUNTER_START_TIMER(GL_CLIENT_WAIT_TIME);
	_get().wait();
	ANKI_COUNTER_STOP_TIMER_INC(GL_CLIENT_WAIT_TIME);
}

} // end namespace anki

