// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/RenderingThread.h"

namespace anki {

//==============================================================================
GrManagerImpl::~GrManagerImpl()
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
Error GrManagerImpl::create(GrManagerInitializer& init)
{
	Error err = ErrorCode::NONE;

	// Create thread
	m_thread = 
		m_manager->getAllocator().newInstance<RenderingThread>(m_manager);

	// Start it
	if(!err)
	{
		err = m_thread->start(init.m_makeCurrentCallback, 
			init.m_makeCurrentCallbackData, init.m_ctx, 
			init.m_swapBuffersCallback, init.m_swapBuffersCallbackData,
			init.m_registerDebugMessages);
	}

	if(!err)
	{
		m_thread->syncClientServer();
	}

	return err;
}

} // end namespace anki

