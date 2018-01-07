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

#include <anki/gr/Buffer.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/Shader.h>
#include <anki/gr/ShaderProgram.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/RenderGraph.h>

#include <cstring>

namespace anki
{

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
	ANKI_ASSERT(gr);

	auto alloc = gr->m_alloc;
	gr->~GrManager();
	alloc.deallocate(gr, 1);
}

void GrManager::beginFrame()
{
	// Nothing for GL
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

BufferPtr GrManager::newBuffer(const BufferInitInfo& init)
{
	return BufferPtr(Buffer::newInstance(this, init));
}

TexturePtr GrManager::newTexture(const TextureInitInfo& init)
{
	return TexturePtr(Texture::newInstance(this, init));
}

TextureViewPtr GrManager::newTextureView(const TextureViewInitInfo& init)
{
	return TextureViewPtr(TextureView::newInstance(this, init));
}

SamplerPtr GrManager::newSampler(const SamplerInitInfo& init)
{
	return SamplerPtr(Sampler::newInstance(this, init));
}

ShaderPtr GrManager::newShader(const ShaderInitInfo& init)
{
	return ShaderPtr(Shader::newInstance(this, init));
}

ShaderProgramPtr GrManager::newShaderProgram(const ShaderProgramInitInfo& init)
{
	return ShaderProgramPtr(ShaderProgram::newInstance(this, init));
}

CommandBufferPtr GrManager::newCommandBuffer(const CommandBufferInitInfo& init)
{
	return CommandBufferPtr(CommandBuffer::newInstance(this, init));
}

FramebufferPtr GrManager::newFramebuffer(const FramebufferInitInfo& init)
{
	return FramebufferPtr(Framebuffer::newInstance(this, init));
}

OcclusionQueryPtr GrManager::newOcclusionQuery()
{
	return OcclusionQueryPtr(OcclusionQuery::newInstance(this));
}

RenderGraphPtr GrManager::newRenderGraph()
{
	return RenderGraphPtr(RenderGraph::newInstance(this));
}

void GrManager::getTextureSurfaceUploadInfo(TexturePtr tex, const TextureSurfaceInfo& surf, PtrSize& allocationSize)
{
	const TextureImpl& impl = static_cast<const TextureImpl&>(*tex);
	// TODO impl.checkSurfaceOrVolume(surf);

	U width = impl.m_width >> surf.m_level;
	U height = impl.m_height >> surf.m_level;
	allocationSize = computeSurfaceSize(width, height, impl.m_format);
}

void GrManager::getTextureVolumeUploadInfo(TexturePtr tex, const TextureVolumeInfo& vol, PtrSize& allocationSize)
{
	const TextureImpl& impl = static_cast<const TextureImpl&>(*tex);
	// TODO impl.checkSurfaceOrVolume(vol);

	U width = impl.m_width >> vol.m_level;
	U height = impl.m_height >> vol.m_level;
	U depth = impl.m_depth >> vol.m_level;
	allocationSize = computeVolumeSize(width, height, depth, impl.m_format);
}

void GrManager::getUniformBufferInfo(U32& bindOffsetAlignment, PtrSize& maxUniformBlockSize) const
{
	ANKI_GL_SELF_CONST(GrManagerImpl);

	bindOffsetAlignment = self.getState().m_uboAlignment;
	maxUniformBlockSize = self.getState().m_uniBlockMaxSize;

	ANKI_ASSERT(bindOffsetAlignment > 0 && maxUniformBlockSize > 0);
}

void GrManager::getStorageBufferInfo(U32& bindOffsetAlignment, PtrSize& maxStorageBlockSize) const
{
	ANKI_GL_SELF_CONST(GrManagerImpl);

	bindOffsetAlignment = self.getState().m_ssboAlignment;
	maxStorageBlockSize = self.getState().m_storageBlockMaxSize;

	ANKI_ASSERT(bindOffsetAlignment > 0 && maxStorageBlockSize > 0);
}

void GrManager::getTextureBufferInfo(U32& bindOffsetAlignment, PtrSize& maxRange) const
{
	ANKI_GL_SELF_CONST(GrManagerImpl);

	bindOffsetAlignment = self.getState().m_tboAlignment;
	maxRange = self.getState().m_tboMaxRange;

	ANKI_ASSERT(bindOffsetAlignment > 0 && maxRange > 0);
}

} // end namespace anki
