// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/TransientMemoryManager.h>
#include <anki/gr/gl/TextureImpl.h>
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
	// Destroy in reverse order
	m_impl.reset(nullptr);
	m_cacheDir.destroy(m_alloc);
}

//==============================================================================
Error GrManager::init(GrManagerInitInfo& init)
{
	m_alloc =
		HeapAllocator<U8>(init.m_allocCallback, init.m_allocCallbackUserData);

	m_cacheDir.create(m_alloc, init.m_cacheDirectory);

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
	// Nothing for GL
}

//==============================================================================
void GrManager::swapBuffers()
{
	m_impl->getRenderingThread().swapBuffers();
}

//==============================================================================
void GrManager::finish()
{
	m_impl->getRenderingThread().syncClientServer();
}

//==============================================================================
void* GrManager::allocateFrameTransientMemory(
	PtrSize size, BufferUsageBit usage, TransientMemoryToken& token)
{
	void* data = nullptr;
	m_impl->getTransientMemoryManager().allocate(size,
		usage,
		TransientMemoryTokenLifetime::PER_FRAME,
		token,
		data,
		nullptr);

	return data;
}

//==============================================================================
void* GrManager::tryAllocateFrameTransientMemory(
	PtrSize size, BufferUsageBit usage, TransientMemoryToken& token)
{
	void* data = nullptr;
	Error err = ErrorCode::NONE;
	m_impl->getTransientMemoryManager().allocate(size,
		usage,
		TransientMemoryTokenLifetime::PER_FRAME,
		token,
		data,
		&err);

	return (!err) ? data : nullptr;
}

//==============================================================================
void GrManager::getTextureSurfaceUploadInfo(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	PtrSize& allocationSize,
	BufferUsageBit& usage)
{
	const TextureImpl& impl = tex->getImplementation();
	impl.checkSurface(surf);

	U width = impl.m_width >> surf.m_level;
	U height = impl.m_height >> surf.m_level;
	allocationSize = computeSurfaceSize(width, height, impl.m_pformat);

	usage = BufferUsageBit::TRANSFER_SOURCE;
}

//==============================================================================
void GrManager::getTextureVolumeUploadInfo(TexturePtr tex,
	const TextureVolumeInfo& vol,
	PtrSize& allocationSize,
	BufferUsageBit& usage)
{
	const TextureImpl& impl = tex->getImplementation();
	impl.checkVolume(vol);

	U width = impl.m_width >> vol.m_level;
	U height = impl.m_height >> vol.m_level;
	U depth = impl.m_depth >> vol.m_level;
	allocationSize = computeVolumeSize(width, height, depth, impl.m_pformat);

	usage = BufferUsageBit::TRANSFER_SOURCE;
}

} // end namespace anki
