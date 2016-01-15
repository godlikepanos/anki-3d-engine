// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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
Error GrManager::create(Initializer& init)
{
	m_alloc =
		HeapAllocator<U8>(init.m_allocCallback, init.m_allocCallbackUserData);

	m_cacheDir.create(m_alloc, init.m_cacheDirectory);
	m_impl.reset(m_alloc.newInstance<GrManagerImpl>(this));
	m_impl->create(init);

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

	void* data = state.allocateDynamicMemory(size, usage, token);
	ANKI_ASSERT(data);

	// Encode token
	PtrSize offset =
		static_cast<U8*>(data) - state.m_dynamicBuffers[usage].m_address;
	ANKI_ASSERT(offset < MAX_U32 && size < MAX_U32);
	token.m_offset = offset;
	token.m_range = size;

	return data;
}

//==============================================================================
void GrManager::finish()
{
	m_impl->getRenderingThread().syncClientServer();
}

} // end namespace anki
