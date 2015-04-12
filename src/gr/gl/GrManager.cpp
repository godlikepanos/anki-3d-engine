// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/GrManager.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"
#include "anki/core/Timestamp.h"
#include <cstring>

namespace anki {

//==============================================================================
Error GrManager::create(Initializer& init)
{
	m_alloc = HeapAllocator<U8>(
		init.m_allocCallback, init.m_allocCallbackUserData);

	m_cacheDir.create(m_alloc, init.m_cacheDirectory);
	m_impl = m_alloc.newInstance<GrManagerImpl>(this);
	Error err = m_impl->create(init);

	return err;
}

//==============================================================================
void GrManager::destroy()
{
	m_cacheDir.destroy(m_alloc);

	if(m_impl)
	{
		m_alloc.deleteInstance(m_impl);
	}
}

//==============================================================================
void GrManager::swapBuffers()
{
	m_impl->getRenderingThread().swapBuffers();
}

//==============================================================================
PtrSize GrManager::getBufferOffsetAlignment(GLenum target) const
{
	const GlState& state = m_impl->getRenderingThread().getState();

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
