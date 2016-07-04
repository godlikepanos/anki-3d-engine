// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

//==============================================================================
GrManager::GrManager()
{
}

//==============================================================================
GrManager::~GrManager()
{
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

	m_impl.reset(m_alloc.newInstance<GrManagerImpl>(this));
	ANKI_CHECK(m_impl->init(init));

	return ErrorCode::NONE;
}

//==============================================================================
void GrManager::beginFrame()
{
	m_impl->beginFrame();
}

//==============================================================================
void GrManager::swapBuffers()
{
	m_impl->endFrame();
}

//==============================================================================
void GrManager::finish()
{
}

//==============================================================================
void* GrManager::allocateFrameTransientMemory(
	PtrSize size, BufferUsage usage, TransientMemoryToken& token, Error* err)
{
	void* ptr = nullptr;
	m_impl->getTransientMemoryManager().allocate(size, usage, token, ptr, err);
	return ptr;
}

} // end namespace anki
