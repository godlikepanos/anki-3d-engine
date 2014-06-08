// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlManager.h"
#include "anki/core/Timestamp.h"

namespace anki {

//==============================================================================
GlManager::GlManager(
	GlCallback makeCurrentCallback, void* context,
	GlCallback swapBuffersCallback, void* swapBuffersCbData,
	Bool registerDebugMessages,
	AllocAlignedCallback alloc, void* allocUserData)
{
	m_alloc = HeapAllocator<U8>(HeapMemoryPool(alloc, allocUserData));

	// Start the server
	m_jobManager = m_alloc.newInstance<GlJobManager>(
		this, alloc, allocUserData);

	m_jobManager->start(makeCurrentCallback, context, 
		swapBuffersCallback, swapBuffersCbData, 
		registerDebugMessages);

	syncClientServer();
}

//==============================================================================
void GlManager::destroy()
{
	if(m_jobManager)
	{
		m_jobManager->stop();
		m_alloc.deleteInstance(m_jobManager);
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
