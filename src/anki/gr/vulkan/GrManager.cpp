// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

#include <anki/gr/Buffer.h>
#include <anki/gr/Texture.h>
#include <anki/gr/TextureView.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/Shader.h>
#include <anki/gr/ShaderProgram.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/RenderGraph.h>

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
	auto alloc = HeapAllocator<U8>(init.m_allocCallback, init.m_allocCallbackUserData);

	GrManagerImpl* impl = alloc.newInstance<GrManagerImpl>();

	// Init
	impl->m_alloc = alloc;
	impl->m_cacheDir.create(alloc, init.m_cacheDirectory);
	Error err = impl->init(init);

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
	ANKI_VK_SELF(GrManagerImpl);
	self.beginFrame();
}

void GrManager::swapBuffers()
{
	ANKI_VK_SELF(GrManagerImpl);
	self.endFrame();
}

void GrManager::finish()
{
	ANKI_ASSERT(!"TODO");
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
	impl.checkSurfaceOrVolume(surf);

	U width = impl.m_width >> surf.m_level;
	U height = impl.m_height >> surf.m_level;

	if(!impl.m_workarounds)
	{
		allocationSize = computeSurfaceSize(width, height, impl.m_format);
	}
	else if(!!(impl.m_workarounds & TextureImplWorkaround::R8G8B8_TO_R8G8B8A8))
	{
		// Extra size for staging buffer
		allocationSize =
			computeSurfaceSize(width, height, PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM));
		alignRoundUp(16, allocationSize);
		allocationSize +=
			computeSurfaceSize(width, height, PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM));
	}
	else
	{
		ANKI_ASSERT(0);
	}
}

void GrManager::getTextureVolumeUploadInfo(TexturePtr tex, const TextureVolumeInfo& vol, PtrSize& allocationSize)
{
	const TextureImpl& impl = static_cast<const TextureImpl&>(*tex);
	impl.checkSurfaceOrVolume(vol);

	U width = impl.m_width >> vol.m_level;
	U height = impl.m_height >> vol.m_level;
	U depth = impl.m_depth >> vol.m_level;

	if(!impl.m_workarounds)
	{
		allocationSize = computeVolumeSize(width, height, depth, impl.m_format);
	}
	else if(!!(impl.m_workarounds & TextureImplWorkaround::R8G8B8_TO_R8G8B8A8))
	{
		// Extra size for staging buffer
		allocationSize =
			computeVolumeSize(width, height, depth, PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM));
		alignRoundUp(16, allocationSize);
		allocationSize +=
			computeVolumeSize(width, height, depth, PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM));
	}
	else
	{
		ANKI_ASSERT(0);
	}
}

void GrManager::getUniformBufferInfo(U32& bindOffsetAlignment, PtrSize& maxUniformBlockSize) const
{
	ANKI_VK_SELF_CONST(GrManagerImpl);
	bindOffsetAlignment = self.getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	maxUniformBlockSize = self.getPhysicalDeviceProperties().limits.maxUniformBufferRange;
}

void GrManager::getStorageBufferInfo(U32& bindOffsetAlignment, PtrSize& maxStorageBlockSize) const
{
	ANKI_VK_SELF_CONST(GrManagerImpl);
	bindOffsetAlignment = self.getPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;
	maxStorageBlockSize = self.getPhysicalDeviceProperties().limits.maxStorageBufferRange;
}

void GrManager::getTextureBufferInfo(U32& bindOffsetAlignment, PtrSize& maxRange) const
{
	ANKI_VK_SELF_CONST(GrManagerImpl);
	bindOffsetAlignment = self.getPhysicalDeviceProperties().limits.minTexelBufferOffsetAlignment;
	maxRange = MAX_U32;
}

} // end namespace anki
