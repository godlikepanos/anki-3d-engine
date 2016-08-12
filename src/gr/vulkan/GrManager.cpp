// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/Texture.h>

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
	PtrSize size, BufferUsageBit usage, TransientMemoryToken& token)
{
	void* ptr = nullptr;
	m_impl->getTransientMemoryManager().allocate(
		size, usage, token, ptr, nullptr);
	return ptr;
}

//==============================================================================
void* GrManager::tryAllocateFrameTransientMemory(
	PtrSize size, BufferUsageBit usage, TransientMemoryToken& token)
{
	void* ptr = nullptr;
	Error err = ErrorCode::NONE;
	m_impl->getTransientMemoryManager().allocate(size, usage, token, ptr, &err);
	return (!err) ? ptr : nullptr;
}

//==============================================================================
void GrManager::getTextureUploadInfo(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	PtrSize& allocationSize,
	BufferUsageBit& usage)
{
	const TextureImpl& impl = tex->getImplementation();
	impl.checkSurface(surf);

	U width = impl.m_width >> surf.m_level;
	U height = impl.m_height >> surf.m_level;

	if(!impl.m_workarounds)
	{
		allocationSize = computeSurfaceSize(width, height, impl.m_format);
	}
	else if(!!(impl.m_workarounds & TextureImplWorkaround::R8G8B8_TO_R8G8B8A8))
	{
		// Extra size for staging buffer
		allocationSize = computeSurfaceSize(width,
			height,
			PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM));
		alignRoundUp(16, allocationSize);
		allocationSize += computeSurfaceSize(width,
			height,
			PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM));
	}
	else
	{
		ANKI_ASSERT(0);
	}

	usage = BufferUsageBit::TRANSFER_SOURCE;
}

} // end namespace anki
