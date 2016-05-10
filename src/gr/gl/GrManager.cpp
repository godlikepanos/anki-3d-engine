// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/core/Timestamp.h>
#include <cstring>

namespace anki
{

//==============================================================================
GrManager::GrManager()
{
}

//==============================================================================
GrManager::~GrManager()
{
	m_cacheDir.destroy(m_alloc);
}

//==============================================================================
Error GrManager::init(GrManagerInitInfo& init)
{
	m_alloc =
		HeapAllocator<U8>(init.m_allocCallback, init.m_allocCallbackUserData);

	for(auto& c : m_caches)
	{
		c.init(m_alloc);
	}

	m_cacheDir.create(m_alloc, init.m_cacheDirectory);
	m_impl.reset(m_alloc.newInstance<GrManagerImpl>(this));
	ANKI_CHECK(m_impl->init(init));

	return ErrorCode::NONE;
}

//==============================================================================
void GrManager::swapBuffers()
{
	m_impl->getRenderingThread().swapBuffers();
}

//==============================================================================
void* GrManager::allocateFrameHostVisibleMemory(
	PtrSize size, BufferUsage usage, DynamicBufferToken& token)
{
	// Will be used in a thread safe way
	GlState& state = m_impl->getRenderingThread().getState();
	void* ptr = state.allocateDynamicMemory(size, usage, token);
	return ptr;
}

//==============================================================================
void GrManager::finish()
{
	m_impl->getRenderingThread().syncClientServer();
}

} // end namespace anki
