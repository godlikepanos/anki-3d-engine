// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/TextureView.h>
#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/Shader.h>
#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Framebuffer.h>
#include <AnKi/Gr/OcclusionQuery.h>
#include <AnKi/Gr/TimestampQuery.h>
#include <AnKi/Gr/RenderGraph.h>
#include <AnKi/Gr/AccelerationStructure.h>
#include <AnKi/Gr/GrUpscaler.h>

namespace anki {

GrManager::GrManager()
{
}

GrManager::~GrManager()
{
	// Destroy in reverse order
	m_cacheDir.destroy(m_pool);
}

Error GrManager::newInstance(GrManagerInitInfo& init, GrManager*& gr)
{
	GrManagerImpl* impl = static_cast<GrManagerImpl*>(
		init.m_allocCallback(init.m_allocCallbackUserData, nullptr, sizeof(GrManagerImpl), alignof(GrManagerImpl)));
	callConstructor(*impl);

	// Init
	impl->m_pool.init(init.m_allocCallback, init.m_allocCallbackUserData);
	impl->m_cacheDir.create(impl->m_pool, init.m_cacheDirectory);
	Error err = impl->init(init);

	if(err)
	{
		callDestructor(*impl);
		init.m_allocCallback(init.m_allocCallbackUserData, impl, 0, 0);
		gr = nullptr;
	}
	else
	{
		gr = impl;
	}

	return err;
}

void GrManager::deleteInstance(GrManager* gr)
{
	if(gr == nullptr)
	{
		return;
	}

	AllocAlignedCallback callback = gr->m_pool.getAllocationCallback();
	void* userData = gr->m_pool.getAllocationCallbackUserData();
	gr->~GrManager();
	callback(userData, gr, 0, 0);
}

TexturePtr GrManager::acquireNextPresentableTexture()
{
	ANKI_VK_SELF(GrManagerImpl);
	return self.acquireNextPresentableTexture();
}

void GrManager::swapBuffers()
{
	ANKI_VK_SELF(GrManagerImpl);
	self.endFrame();
}

void GrManager::finish()
{
	ANKI_VK_SELF(GrManagerImpl);
	self.finish();
}

GrManagerStats GrManager::getStats() const
{
	ANKI_VK_SELF_CONST(GrManagerImpl);
	GrManagerStats out;

	GpuMemoryManagerStats memStats;
	self.getGpuMemoryManager().getStats(memStats);

	out.m_deviceMemoryAllocated = memStats.m_deviceMemoryAllocated;
	out.m_deviceMemoryInUse = memStats.m_deviceMemoryInUse;
	out.m_deviceMemoryAllocationCount = memStats.m_deviceMemoryAllocationCount;
	out.m_hostMemoryAllocated = memStats.m_hostMemoryAllocated;
	out.m_hostMemoryInUse = memStats.m_hostMemoryInUse;
	out.m_hostMemoryAllocationCount = memStats.m_hostMemoryAllocationCount;

	out.m_commandBufferCount = self.getCommandBufferFactory().getCreatedCommandBufferCount();

	return out;
}

#define ANKI_NEW_GR_OBJECT(type) \
	type##Ptr GrManager::new##type(const type##InitInfo& init) \
	{ \
		type##Ptr ptr(type::newInstance(this, init)); \
		if(ANKI_UNLIKELY(!ptr.isCreated())) \
		{ \
			ANKI_VK_LOGF("Failed to create a " ANKI_STRINGIZE(type) " object"); \
		} \
		return ptr; \
	}

#define ANKI_NEW_GR_OBJECT_NO_INIT_INFO(type) \
	type##Ptr GrManager::new##type() \
	{ \
		type##Ptr ptr(type::newInstance(this)); \
		if(ANKI_UNLIKELY(!ptr.isCreated())) \
		{ \
			ANKI_VK_LOGF("Failed to create a " ANKI_STRINGIZE(type) " object"); \
		} \
		return ptr; \
	}

ANKI_NEW_GR_OBJECT(Buffer)
ANKI_NEW_GR_OBJECT(Texture)
ANKI_NEW_GR_OBJECT(TextureView)
ANKI_NEW_GR_OBJECT(Sampler)
ANKI_NEW_GR_OBJECT(Shader)
ANKI_NEW_GR_OBJECT(ShaderProgram)
ANKI_NEW_GR_OBJECT(CommandBuffer)
ANKI_NEW_GR_OBJECT(Framebuffer)
ANKI_NEW_GR_OBJECT_NO_INIT_INFO(OcclusionQuery)
ANKI_NEW_GR_OBJECT_NO_INIT_INFO(TimestampQuery)
ANKI_NEW_GR_OBJECT_NO_INIT_INFO(RenderGraph)
ANKI_NEW_GR_OBJECT(AccelerationStructure)
ANKI_NEW_GR_OBJECT(GrUpscaler)

#undef ANKI_NEW_GR_OBJECT
#undef ANKI_NEW_GR_OBJECT_NO_INIT_INFO

} // end namespace anki
