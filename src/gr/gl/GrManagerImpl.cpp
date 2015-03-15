// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"

namespace anki {

//==============================================================================
Error GlManagerImpl::create(GrManagerInitializer& init)
{
	Error err = ErrorCode::NONE;

	// Create thread
	m_thread = getAllocator().newInstance<RenderingThread>(m_manager);
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

//==============================================================================
GrAllocator<U8> GlManagerImpl::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki

