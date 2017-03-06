// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/gl/GlState.h>
#include <anki/core/Timestamp.h>
#include <cstring>

namespace anki
{

GrManager::GrManager()
{
}

GrManager::~GrManager()
{
	// Destroy in reverse order
	m_impl.reset(nullptr);
	m_cacheDir.destroy(m_alloc);
}

Error GrManager::init(GrManagerInitInfo& init)
{
	m_alloc = HeapAllocator<U8>(init.m_allocCallback, init.m_allocCallbackUserData);

	m_cacheDir.create(m_alloc, init.m_cacheDirectory);

	m_impl.reset(m_alloc.newInstance<GrManagerImpl>(this));
	ANKI_CHECK(m_impl->init(init));

	return ErrorCode::NONE;
}

void GrManager::beginFrame()
{
	// Nothing for GL
}

void GrManager::swapBuffers()
{
	m_impl->getRenderingThread().swapBuffers();
}

void GrManager::finish()
{
	m_impl->getRenderingThread().syncClientServer();
}

void GrManager::getTextureSurfaceUploadInfo(TexturePtr tex, const TextureSurfaceInfo& surf, PtrSize& allocationSize)
{
	const TextureImpl& impl = *tex->m_impl;
	impl.checkSurfaceOrVolume(surf);

	U width = impl.m_width >> surf.m_level;
	U height = impl.m_height >> surf.m_level;
	allocationSize = computeSurfaceSize(width, height, impl.m_pformat);
}

void GrManager::getTextureVolumeUploadInfo(TexturePtr tex, const TextureVolumeInfo& vol, PtrSize& allocationSize)
{
	const TextureImpl& impl = *tex->m_impl;
	impl.checkSurfaceOrVolume(vol);

	U width = impl.m_width >> vol.m_level;
	U height = impl.m_height >> vol.m_level;
	U depth = impl.m_depth >> vol.m_level;
	allocationSize = computeVolumeSize(width, height, depth, impl.m_pformat);
}

void GrManager::getUniformBufferInfo(U32& bindOffsetAlignment, PtrSize& maxUniformBlockSize) const
{
	bindOffsetAlignment = m_impl->getState().m_uboAlignment;
	maxUniformBlockSize = m_impl->getState().m_uniBlockMaxSize;

	ANKI_ASSERT(bindOffsetAlignment > 0 && maxUniformBlockSize > 0);
}

void GrManager::getStorageBufferInfo(U32& bindOffsetAlignment, PtrSize& maxStorageBlockSize) const
{
	bindOffsetAlignment = m_impl->getState().m_ssboAlignment;
	maxStorageBlockSize = m_impl->getState().m_storageBlockMaxSize;

	ANKI_ASSERT(bindOffsetAlignment > 0 && maxStorageBlockSize > 0);
}

void GrManager::getTextureBufferInfo(U32& bindOffsetAlignment, PtrSize& maxRange) const
{
	bindOffsetAlignment = m_impl->getState().m_tboAlignment;
	maxRange = m_impl->getState().m_tboMaxRange;

	ANKI_ASSERT(bindOffsetAlignment > 0 && maxRange > 0);
}

} // end namespace anki
