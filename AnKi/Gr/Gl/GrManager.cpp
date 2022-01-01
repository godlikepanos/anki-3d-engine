// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/gl/GrManagerImpl.h>
#include <AnKi/Gr/gl/RenderingThread.h>
#include <AnKi/Gr/gl/TextureImpl.h>
#include <AnKi/Gr/gl/GlState.h>

#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/Shader.h>
#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Framebuffer.h>
#include <AnKi/Gr/OcclusionQuery.h>
#include <AnKi/Gr/RenderGraph.h>

#include <cstring>

namespace anki {

GrManager::GrManager()
{
}

GrManager::~GrManager()
{
	// Destroy in reverse order
	m_cacheDir.destroy(m_alloc);
}

Error GrManager::newInstance(GrManagerInitInfo& init, GrManager*& gr)
{
	auto alloc = GrAllocator<U8>(init.m_allocCallback, init.m_allocCallbackUserData);

	GrManagerImpl* impl = alloc.newInstance<GrManagerImpl>();
	Error err = impl->init(init, alloc);

	if(err)
	{
		alloc.deleteInstance(impl);
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

	auto alloc = gr->m_alloc;
	gr->~GrManager();
	alloc.deallocate(gr, 1);
}

TexturePtr GrManager::acquireNextPresentableTexture()
{
	ANKI_GL_SELF(GrManagerImpl);
	return self.m_fakeFbTex;
}

void GrManager::swapBuffers()
{
	ANKI_GL_SELF(GrManagerImpl);
	self.getRenderingThread().swapBuffers();
}

void GrManager::finish()
{
	ANKI_GL_SELF(GrManagerImpl);
	self.getRenderingThread().syncClientServer();
}

#define ANKI_SAFE_CONSTRUCT(class_) \
	class_* out = class_::newInstance(this, init); \
	class_##Ptr ptr(out); \
	out->getRefcount().fetchSub(1); \
	return ptr

BufferPtr GrManager::newBuffer(const BufferInitInfo& init)
{
	ANKI_SAFE_CONSTRUCT(Buffer);
}

TexturePtr GrManager::newTexture(const TextureInitInfo& init)
{
	ANKI_SAFE_CONSTRUCT(Texture);
}

TextureViewPtr GrManager::newTextureView(const TextureViewInitInfo& init)
{
	ANKI_SAFE_CONSTRUCT(TextureView);
}

SamplerPtr GrManager::newSampler(const SamplerInitInfo& init)
{
	ANKI_SAFE_CONSTRUCT(Sampler);
}

ShaderPtr GrManager::newShader(const ShaderInitInfo& init)
{
	ANKI_SAFE_CONSTRUCT(Shader);
}

ShaderProgramPtr GrManager::newShaderProgram(const ShaderProgramInitInfo& init)
{
	ANKI_SAFE_CONSTRUCT(ShaderProgram);
}

CommandBufferPtr GrManager::newCommandBuffer(const CommandBufferInitInfo& init)
{
	return CommandBufferPtr(CommandBuffer::newInstance(this, init));
}

FramebufferPtr GrManager::newFramebuffer(const FramebufferInitInfo& init)
{
	ANKI_SAFE_CONSTRUCT(Framebuffer);
}

OcclusionQueryPtr GrManager::newOcclusionQuery()
{
	OcclusionQuery* out = OcclusionQuery::newInstance(this);
	OcclusionQueryPtr ptr(out);
	out->getRefcount().fetchSub(1);
	return ptr;
}

RenderGraphPtr GrManager::newRenderGraph()
{
	return RenderGraphPtr(RenderGraph::newInstance(this));
}

#undef ANKI_SAFE_CONSTRUCT

GrManagerStats GrManager::getStats() const
{
	return GrManagerStats();
}

} // end namespace anki
