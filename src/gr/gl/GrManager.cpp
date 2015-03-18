// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/GrManager.h"
#include "anki/core/Timestamp.h"
#include <cstring>

namespace anki {

//==============================================================================
Error GlDevice::create(Initializer& init)
{
	m_alloc = HeapAllocator<U8>(alloc, allocUserData);

	Error err = m_cacheDir.create(m_alloc, cacheDir);

	if(!err)
	{
		m_impl = m_alloc.newInstance<GrManagerImpl>(this);
	}

	if(m_impl)
	{
		err = m_impl->create(init);
	}
	else
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	return ErrorCode::NONE;
}

//==============================================================================
void GlDevice::destroy()
{
	if(m_impl)
	{
		m_alloc.deleteInstance(m_impl);
	}

	if(m_cacheDir)
	{
		m_alloc.deallocate(m_cacheDir, std::strlen(m_cacheDir) + 1);
	}
}

//==============================================================================
void GlDevice::swapBuffers()
{
	m_queue->swapBuffers();
}

//==============================================================================
PtrSize GlDevice::getBufferOffsetAlignment(GLenum target) const
{
	const State& state = m_queue->getState();

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
