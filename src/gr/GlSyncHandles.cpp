// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/GlSyncHandles.h"
#include "anki/gr/GlSync.h"
#include "anki/gr/CommandBufferHandle.h"
#include "anki/gr/GlDevice.h"
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

	Error operator()(CommandBufferImpl*)
	{
		ANKI_COUNTER_START_TIMER(GL_SERVER_WAIT_TIME);
		m_sync._get().wait();
		ANKI_COUNTER_STOP_TIMER_INC(GL_SERVER_WAIT_TIME);

		return ErrorCode::NONE;
	}
};

//==============================================================================
GlClientSyncHandle::GlClientSyncHandle()
{}

//==============================================================================
GlClientSyncHandle::~GlClientSyncHandle()
{}

//==============================================================================
Error GlClientSyncHandle::create(CommandBufferHandle& commands)
{
	auto alloc = commands._getGlobalAllocator();

	using Deleter = 
		GlHandleDefaultDeleter<GlClientSync, GlAllocator<U8>>;

	return _createAdvanced(
		&commands._getQueue().getDevice(), alloc, Deleter());
}

//==============================================================================
void GlClientSyncHandle::sync(CommandBufferHandle& commands)
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

