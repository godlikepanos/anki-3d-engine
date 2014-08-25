// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlDevice.h"
#include "anki/core/Timestamp.h"

namespace anki {

//==============================================================================
GlDevice::GlDevice(
	GlCallback makeCurrentCallback, void* context,
	GlCallback swapBuffersCallback, void* swapBuffersCbData,
	Bool registerDebugMessages,
	AllocAlignedCallback alloc, void* allocUserData)
{
	m_alloc = HeapAllocator<U8>(HeapMemoryPool(alloc, allocUserData));

	// Start the server
	m_queue = m_alloc.newInstance<GlQueue>(
		this, alloc, allocUserData);

	m_queue->start(makeCurrentCallback, context, 
		swapBuffersCallback, swapBuffersCbData, 
		registerDebugMessages);

	syncClientServer();
}

//==============================================================================
void GlDevice::destroy()
{
	if(m_queue)
	{
		m_queue->stop();
		m_alloc.deleteInstance(m_queue);
	}
}

//==============================================================================
void GlDevice::syncClientServer()
{
	m_queue->syncClientServer();
}

//==============================================================================
void GlDevice::swapBuffers()
{
	m_queue->swapBuffers();
}

//==============================================================================
PtrSize GlDevice::getBufferOffsetAlignment(GLenum target)
{
	const GlState& state = m_queue->getState();

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
