// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/RenderGraph.h>

namespace anki
{

inline void RenderPassWorkContext::bindAccelerationStructure(U32 set, U32 binding, AccelerationStructureHandle handle)
{
	m_commandBuffer->bindAccelerationStructure(set, binding, m_rgraph->getAs(handle));
}

inline void RenderPassWorkContext::getBufferState(BufferHandle handle, BufferPtr& buff) const
{
	buff = m_rgraph->getBuffer(handle);
}

inline void RenderPassWorkContext::getRenderTargetState(RenderTargetHandle handle,
														const TextureSubresourceInfo& subresource, TexturePtr& tex,
														TextureUsageBit& usage) const
{
	m_rgraph->getCrntUsage(handle, m_batchIdx, subresource, usage);
	tex = m_rgraph->getTexture(handle);
}

inline TexturePtr RenderPassWorkContext::getTexture(RenderTargetHandle handle) const
{
	return m_rgraph->getTexture(handle);
}

inline void RenderPassDescriptionBase::fixSubresource(RenderPassDependency& dep) const
{
	ANKI_ASSERT(dep.m_type == RenderPassDependency::Type::TEXTURE);

	TextureSubresourceInfo& subresource = dep.m_texture.m_subresource;
	const Bool wholeTexture = subresource.m_mipmapCount == MAX_U32;
	if(wholeTexture)
	{
		ANKI_ASSERT(subresource.m_firstFace == 0);
		ANKI_ASSERT(subresource.m_firstMipmap == 0);
		ANKI_ASSERT(subresource.m_firstLayer == 0);

		const RenderGraphDescription::RT& rt = m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx];
		if(rt.m_importedTex)
		{
			subresource.m_faceCount = textureTypeIsCube(rt.m_importedTex->getTextureType()) ? 6 : 1;
			subresource.m_mipmapCount = rt.m_importedTex->getMipmapCount();
			subresource.m_layerCount = rt.m_importedTex->getLayerCount();
		}
		else
		{
			subresource.m_faceCount = textureTypeIsCube(rt.m_initInfo.m_type) ? 6 : 1;
			subresource.m_mipmapCount = rt.m_initInfo.m_mipmapCount;
			subresource.m_layerCount = rt.m_initInfo.m_layerCount;
		}
	}
}

inline void RenderPassDescriptionBase::validateDep(const RenderPassDependency& dep)
{
	// Validate dep
	if(dep.m_type == RenderPassDependency::Type::TEXTURE)
	{
		const TextureUsageBit usage = dep.m_texture.m_usage;
		(void)usage;
		if(m_type == Type::GRAPHICS)
		{
			ANKI_ASSERT(!(usage & TextureUsageBit::ALL_COMPUTE));
		}
		else
		{
			ANKI_ASSERT(!(usage & TextureUsageBit::ALL_GRAPHICS));
		}

		ANKI_ASSERT(!!(usage & TextureUsageBit::ALL_READ) || !!(usage & TextureUsageBit::ALL_WRITE));
	}
	else if(dep.m_type == RenderPassDependency::Type::BUFFER)
	{
		const BufferUsageBit usage = dep.m_buffer.m_usage;
		(void)usage;
		if(m_type == Type::GRAPHICS)
		{
			ANKI_ASSERT(!(usage & BufferUsageBit::ALL_COMPUTE));
		}
		else
		{
			ANKI_ASSERT(!(usage & BufferUsageBit::ALL_GRAPHICS));
		}

		ANKI_ASSERT(!!(usage & BufferUsageBit::ALL_READ) || !!(usage & BufferUsageBit::ALL_WRITE));
	}
	else
	{
		ANKI_ASSERT(dep.m_type == RenderPassDependency::Type::ACCELERATION_STRUCTURE);
		if(m_type == Type::GRAPHICS)
		{
			ANKI_ASSERT(!(dep.m_as.m_usage & ~AccelerationStructureUsageBit::ALL_GRAPHICS));
		}
		else
		{
			ANKI_ASSERT(!(dep.m_as.m_usage & AccelerationStructureUsageBit::ALL_GRAPHICS));
		}
	}
}

inline void RenderPassDescriptionBase::newDependency(const RenderPassDependency& dep)
{
	validateDep(dep);

	if(dep.m_type == RenderPassDependency::Type::TEXTURE)
	{
		m_rtDeps.emplaceBack(m_alloc, dep);
		fixSubresource(m_rtDeps.getBack());

		if(!!(dep.m_texture.m_usage & TextureUsageBit::ALL_READ))
		{
			m_readRtMask.set(dep.m_texture.m_handle.m_idx);
		}

		if(!!(dep.m_texture.m_usage & TextureUsageBit::ALL_WRITE))
		{
			m_writeRtMask.set(dep.m_texture.m_handle.m_idx);
		}

		// Try to derive the usage by that dep
		m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx].m_usageDerivedByDeps |= dep.m_texture.m_usage;
	}
	else if(dep.m_type == RenderPassDependency::Type::BUFFER)
	{
		m_buffDeps.emplaceBack(m_alloc, dep);

		if(!!(dep.m_buffer.m_usage & BufferUsageBit::ALL_READ))
		{
			m_readBuffMask.set(dep.m_buffer.m_handle.m_idx);
		}

		if(!!(dep.m_buffer.m_usage & BufferUsageBit::ALL_WRITE))
		{
			m_writeBuffMask.set(dep.m_buffer.m_handle.m_idx);
		}
	}
	else
	{
		ANKI_ASSERT(dep.m_type == RenderPassDependency::Type::ACCELERATION_STRUCTURE);
		m_asDeps.emplaceBack(m_alloc, dep);

		if(!!(dep.m_as.m_usage & AccelerationStructureUsageBit::ALL_READ))
		{
			m_readAsMask.set(dep.m_as.m_handle.m_idx);
		}

		if(!!(dep.m_as.m_usage & AccelerationStructureUsageBit::ALL_WRITE))
		{
			m_writeAsMask.set(dep.m_as.m_handle.m_idx);
		}
	}
}

inline void GraphicsRenderPassDescription::setFramebufferInfo(
	const FramebufferDescription& fbInfo, std::initializer_list<RenderTargetHandle> colorRenderTargetHandles,
	RenderTargetHandle depthStencilRenderTargetHandle, U32 minx, U32 miny, U32 maxx, U32 maxy)
{
	Array<RenderTargetHandle, MAX_COLOR_ATTACHMENTS> rts;
	U32 count = 0;
	for(const RenderTargetHandle& h : colorRenderTargetHandles)
	{
		rts[count++] = h;
	}
	setFramebufferInfo(fbInfo, ConstWeakArray<RenderTargetHandle>(&rts[0], count), depthStencilRenderTargetHandle, minx,
					   miny, maxx, maxy);
}

inline void GraphicsRenderPassDescription::setFramebufferInfo(
	const FramebufferDescription& fbInfo, ConstWeakArray<RenderTargetHandle> colorRenderTargetHandles,
	RenderTargetHandle depthStencilRenderTargetHandle, U32 minx, U32 miny, U32 maxx, U32 maxy)
{
#if ANKI_ENABLE_ASSERTS
	ANKI_ASSERT(fbInfo.isBacked() && "Forgot call GraphicsRenderPassFramebufferInfo::bake");
	for(U32 i = 0; i < colorRenderTargetHandles.getSize(); ++i)
	{
		if(i >= fbInfo.m_colorAttachmentCount)
		{
			ANKI_ASSERT(!colorRenderTargetHandles[i].isValid());
		}
		else
		{
			ANKI_ASSERT(colorRenderTargetHandles[i].isValid());
		}
	}

	if(!fbInfo.m_depthStencilAttachment.m_aspect)
	{
		ANKI_ASSERT(!depthStencilRenderTargetHandle.isValid());
	}
	else
	{
		ANKI_ASSERT(depthStencilRenderTargetHandle.isValid());
	}
#endif

	m_fbDescr = fbInfo;
	memcpy(m_rtHandles.getBegin(), colorRenderTargetHandles.getBegin(), colorRenderTargetHandles.getSizeInBytes());
	m_rtHandles[MAX_COLOR_ATTACHMENTS] = depthStencilRenderTargetHandle;
	m_fbRenderArea = {minx, miny, maxx, maxy};
}

inline RenderGraphDescription::~RenderGraphDescription()
{
	for(RenderPassDescriptionBase* pass : m_passes)
	{
		m_alloc.deleteInstance(pass);
	}
	m_passes.destroy(m_alloc);
	m_renderTargets.destroy(m_alloc);
	m_buffers.destroy(m_alloc);
	m_as.destroy(m_alloc);
}

inline GraphicsRenderPassDescription& RenderGraphDescription::newGraphicsRenderPass(CString name)
{
	GraphicsRenderPassDescription* pass = m_alloc.newInstance<GraphicsRenderPassDescription>(this);
	pass->m_alloc = m_alloc;
	pass->setName(name);
	m_passes.emplaceBack(m_alloc, pass);
	return *pass;
}

inline ComputeRenderPassDescription& RenderGraphDescription::newComputeRenderPass(CString name)
{
	ComputeRenderPassDescription* pass = m_alloc.newInstance<ComputeRenderPassDescription>(this);
	pass->m_alloc = m_alloc;
	pass->setName(name);
	m_passes.emplaceBack(m_alloc, pass);
	return *pass;
}

inline RenderTargetHandle RenderGraphDescription::importRenderTarget(TexturePtr tex, TextureUsageBit usage)
{
	for(const RT& rt : m_renderTargets)
	{
		(void)rt;
		ANKI_ASSERT(rt.m_importedTex != tex && "Already imported");
	}

	RT& rt = *m_renderTargets.emplaceBack(m_alloc);
	rt.m_importedTex = tex;
	rt.m_importedLastKnownUsage = usage;
	rt.m_usageDerivedByDeps = TextureUsageBit::NONE;
	rt.setName(tex->getName());

	RenderTargetHandle out;
	out.m_idx = m_renderTargets.getSize() - 1;
	return out;
}

inline RenderTargetHandle RenderGraphDescription::importRenderTarget(TexturePtr tex)
{
	RenderTargetHandle out = importRenderTarget(tex, TextureUsageBit::NONE);
	m_renderTargets.getBack().m_importedAndUndefinedUsage = true;
	return out;
}

inline RenderTargetHandle RenderGraphDescription::newRenderTarget(const RenderTargetDescription& initInf)
{
	ANKI_ASSERT(initInf.m_hash && "Forgot to call RenderTargetDescription::bake");
	ANKI_ASSERT(initInf.m_usage == TextureUsageBit::NONE && "Don't need to supply the usage. Render grap will find it");
	RT& rt = *m_renderTargets.emplaceBack(m_alloc);
	rt.m_initInfo = initInf;
	rt.m_hash = initInf.m_hash;
	rt.m_importedLastKnownUsage = TextureUsageBit::NONE;
	rt.m_usageDerivedByDeps = TextureUsageBit::NONE;
	rt.setName(initInf.getName());

	RenderTargetHandle out;
	out.m_idx = m_renderTargets.getSize() - 1;
	return out;
}

inline BufferHandle RenderGraphDescription::importBuffer(BufferPtr buff, BufferUsageBit usage)
{
	for(const Buffer& bb : m_buffers)
	{
		(void)bb;
		ANKI_ASSERT(bb.m_importedBuff != buff && "Already imported");
	}

	Buffer& b = *m_buffers.emplaceBack(m_alloc);
	b.setName(buff->getName());
	b.m_usage = usage;
	b.m_importedBuff = buff;

	BufferHandle out;
	out.m_idx = m_buffers.getSize() - 1;
	return out;
}

inline AccelerationStructureHandle
RenderGraphDescription::importAccelerationStructure(AccelerationStructurePtr as, AccelerationStructureUsageBit usage)
{
	for(const AS& a : m_as)
	{
		(void)a;
		ANKI_ASSERT(a.m_importedAs != as && "Already imported");
	}

	AS& a = *m_as.emplaceBack(m_alloc);
	a.setName(as->getName());
	a.m_importedAs = as;
	a.m_usage = usage;

	AccelerationStructureHandle handle;
	handle.m_idx = m_as.getSize() - 1;
	return handle;
}

} // end namespace anki
