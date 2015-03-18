// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"

namespace anki {

//==============================================================================
GrManager::~GrManager()
{
	if(m_thread)
	{
		m_thread->stop();
		m_manager->getAllocator().deleteInstance(m_thread);
		m_thread = nullptr;
	}

	m_manager = nullptr;
}

//==============================================================================
Error GlManagerImpl::create(GrManagerInitializer& init)
{
	Error err = ErrorCode::NONE;

#if ANKI_QUEUE_DISABLE_ASYNC
	ANKI_LOGW("GL queue works in synchronous mode");
#endif

	// Create thread
	m_thread = 
		m_manager->getAllocator().newInstance<RenderingThread>(m_manager);
	if(!thread)
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	// Start it
	if(!err)
	{
		err = m_thread->start(init.m_makeCurrentCallback, 
			init.m_makeCurrentCallbackData, init.m_ctx, 
			init.m_swapBuffersCallback, init.m_swapBuffersCallbackData);
	}

	if(!err)
	{
		m_thread->syncClientServer();
	}

	return err;
}

} // end namespace anki

