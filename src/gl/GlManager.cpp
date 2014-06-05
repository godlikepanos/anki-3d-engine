// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlManager.h"
#include "anki/core/Timestamp.h"

namespace anki {

//==============================================================================
void GlManager::init(
	GlCallback makeCurrentCallback, void* context,
	GlCallback swapBuffersCallback, void* swapBuffersCbData,
	Bool registerDebugMessages)
{
	m_alloc = HeapAllocator<U8>(HeapMemoryPool(0));

	// Start the server
	m_jobManager.reset(new GlJobManager(this));
	m_jobManager->start(makeCurrentCallback, context, 
		swapBuffersCallback, swapBuffersCbData, 
		registerDebugMessages);
	syncClientServer();
}

//==============================================================================
void GlManager::destroy()
{
	if(m_jobManager.get())
	{
		m_jobManager->stop();
		m_jobManager.reset(nullptr);
	}
}

//==============================================================================
void GlManager::syncClientServer()
{
	m_jobManager->syncClientServer();
}

//==============================================================================
void GlManager::swapBuffers()
{
	m_jobManager->swapBuffers();
}

//==============================================================================
PtrSize GlManager::getBufferOffsetAlignment(GLenum target)
{
	const GlState& state = m_jobManager->getState();

	if(target == GL_UNIFORM_BUFFER)
	{
		return state.m_uniBuffOffsetAlignment;
	}
	else
	{
		ANKI_ASSERT(target == GL_SHADER_STORAGE_BUFFER);
		return state.m_ssBuffOffsetAlignment;
	}
}

} // end namespace anki
