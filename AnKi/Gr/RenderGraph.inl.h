// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/RenderGraph.h>

namespace anki {

inline void RenderPassWorkContext::bindAccelerationStructure(U32 set, U32 binding, AccelerationStructureHandle handle)
{
	m_commandBuffer->bindAccelerationStructure(set, binding, m_rgraph->getAs(handle));
}

inline void RenderPassWorkContext::getBufferState(BufferHandle handle, BufferPtr& buff) const
{
	buff = m_rgraph->getBuffer(handle);
}

inline void RenderPassWorkContext::getRenderTargetState(RenderTargetHandle handle,
														const TextureSubresourceInfo& subresource,
														TexturePtr& tex) const
{
	TextureUsageBit usage;
	m_rgraph->getCrntUsage(handle, m_batchIdx, subresource, usage);
	tex = m_rgraph->getTexture(handle);
}

inline TexturePtr RenderPassWorkContext::getTexture(RenderTargetHandle handle) const
{
	return m_rgraph->getTexture(handle);
}

inline void RenderPassDescriptionBase::fixSubresource(RenderPassDependency& dep) const
{
	ANKI_ASSERT(dep.m_type == RenderPassDependency::Type::kTexture);

	TextureSubresourceInfo& subresource = dep.m_texture.m_subresource;
	const Bool wholeTexture = subresource.m_mipmapCount == kMaxU32;
	const RenderGraphDescription::RT& rt = m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx];
	if(wholeTexture)
	{
		ANKI_ASSERT(subresource.m_firstFace == 0);
		ANKI_ASSERT(subresource.m_firstMipmap == 0);
		ANKI_ASSERT(subresource.m_firstLayer == 0);

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

	if(subresource.m_depthStencilAspect == DepthStencilAspectBit::kNone)
	{
		const Bool imported = rt.m_importedTex.isCreated();
		if(imported)
		{
			subresource.m_depthStencilAspect = rt.m_importedTex->getDepthStencilAspect();
		}
		else if(!imported && getFormatInfo(rt.m_initInfo.m_format).isDepthStencil())
		{
			subresource.m_depthStencilAspect = getFormatInfo(rt.m_initInfo.m_format).m_depthStencil;
		}
	}

	ANKI_ASSERT(dep.m_texture.m_subresource.m_firstMipmap + dep.m_texture.m_subresource.m_mipmapCount
				<= ((rt.m_importedTex) ? rt.m_importedTex->getMipmapCount() : rt.m_initInfo.m_mipmapCount));
}

inline void RenderPassDescriptionBase::validateDep(const RenderPassDependency& dep)
{
	// Validate dep
	if(dep.m_type == RenderPassDependency::Type::kTexture)
	{
		[[maybe_unused]] const TextureUsageBit usage = dep.m_texture.m_usage;
		if(m_type == Type::kGraphics)
		{
			ANKI_ASSERT(!(usage & TextureUsageBit::kAllCompute));
		}
		else
		{
			ANKI_ASSERT(!(usage & TextureUsageBit::kAllGraphics));
		}

		ANKI_ASSERT(!!(usage & TextureUsageBit::kAllRead) || !!(usage & TextureUsageBit::kAllWrite));
	}
	else if(dep.m_type == RenderPassDependency::Type::kBuffer)
	{
		[[maybe_unused]] const BufferUsageBit usage = dep.m_buffer.m_usage;
		if(m_type == Type::kGraphics)
		{
			ANKI_ASSERT(!(usage & BufferUsageBit::kAllCompute));
		}
		else
		{
			ANKI_ASSERT(!(usage & BufferUsageBit::kAllGraphics));
		}

		ANKI_ASSERT(!!(usage & BufferUsageBit::kAllRead) || !!(usage & BufferUsageBit::kAllWrite));
	}
	else
	{
		ANKI_ASSERT(dep.m_type == RenderPassDependency::Type::kAccelerationStructure);
		if(m_type == Type::kGraphics)
		{
			ANKI_ASSERT(!(dep.m_as.m_usage & ~AccelerationStructureUsageBit::kAllGraphics));
		}
		else
		{
			ANKI_ASSERT(!(dep.m_as.m_usage & AccelerationStructureUsageBit::kAllGraphics));
		}
	}
}

template<RenderPassDependency::Type kType>
inline void RenderPassDescriptionBase::newDependency(const RenderPassDependency& dep)
{
	ANKI_ASSERT(kType == dep.m_type);
	validateDep(dep);

	if(kType == RenderPassDependency::Type::kTexture)
	{
		m_rtDeps.emplaceBack(*m_pool, dep);
		fixSubresource(m_rtDeps.getBack());

		if(!!(dep.m_texture.m_usage & TextureUsageBit::kAllRead))
		{
			m_readRtMask.set(dep.m_texture.m_handle.m_idx);
		}

		if(!!(dep.m_texture.m_usage & TextureUsageBit::kAllWrite))
		{
			m_writeRtMask.set(dep.m_texture.m_handle.m_idx);
		}

		// Try to derive the usage by that dep
		m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx].m_usageDerivedByDeps |= dep.m_texture.m_usage;

		// Checks
#if ANKI_ENABLE_ASSERTIONS
		const RenderGraphDescription::RT& rt = m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx];
		if((!rt.m_importedTex.isCreated() && !!getFormatInfo(rt.m_initInfo.m_format).m_depthStencil)
		   || (rt.m_importedTex.isCreated() && !!rt.m_importedTex->getDepthStencilAspect()))
		{
			ANKI_ASSERT(!!m_rtDeps.getBack().m_texture.m_subresource.m_depthStencilAspect
						&& "Dependencies of depth/stencil resources should have a valid DS aspect");
		}
#endif
	}
	else if(kType == RenderPassDependency::Type::kBuffer)
	{
		m_buffDeps.emplaceBack(*m_pool, dep);

		if(!!(dep.m_buffer.m_usage & BufferUsageBit::kAllRead))
		{
			m_readBuffMask.set(dep.m_buffer.m_handle.m_idx);
		}

		if(!!(dep.m_buffer.m_usage & BufferUsageBit::kAllWrite))
		{
			m_writeBuffMask.set(dep.m_buffer.m_handle.m_idx);
		}
	}
	else
	{
		ANKI_ASSERT(kType == RenderPassDependency::Type::kAccelerationStructure);
		m_asDeps.emplaceBack(*m_pool, dep);

		if(!!(dep.m_as.m_usage & AccelerationStructureUsageBit::kAllRead))
		{
			m_readAsMask.set(dep.m_as.m_handle.m_idx);
		}

		if(!!(dep.m_as.m_usage & AccelerationStructureUsageBit::kAllWrite))
		{
			m_writeAsMask.set(dep.m_as.m_handle.m_idx);
		}
	}
}

inline void GraphicsRenderPassDescription::setFramebufferInfo(
	const FramebufferDescription& fbInfo, std::initializer_list<RenderTargetHandle> colorRenderTargetHandles,
	RenderTargetHandle depthStencilRenderTargetHandle, RenderTargetHandle shadingRateRenderTargetHandle, U32 minx,
	U32 miny, U32 maxx, U32 maxy)
{
	Array<RenderTargetHandle, kMaxColorRenderTargets> rts;
	U32 count = 0;
	for(const RenderTargetHandle& h : colorRenderTargetHandles)
	{
		rts[count++] = h;
	}
	setFramebufferInfo(fbInfo, ConstWeakArray<RenderTargetHandle>(&rts[0], count), depthStencilRenderTargetHandle,
					   shadingRateRenderTargetHandle, minx, miny, maxx, maxy);
}

inline void GraphicsRenderPassDescription::setFramebufferInfo(
	const FramebufferDescription& fbInfo, ConstWeakArray<RenderTargetHandle> colorRenderTargetHandles,
	RenderTargetHandle depthStencilRenderTargetHandle, RenderTargetHandle shadingRateRenderTargetHandle, U32 minx,
	U32 miny, U32 maxx, U32 maxy)
{
#if ANKI_ENABLE_ASSERTIONS
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

	if(fbInfo.m_shadingRateAttachmentTexelWidth > 0 && fbInfo.m_shadingRateAttachmentTexelHeight > 0)
	{
		ANKI_ASSERT(shadingRateRenderTargetHandle.isValid());
	}
	else
	{
		ANKI_ASSERT(!shadingRateRenderTargetHandle.isValid());
	}
#endif

	m_fbDescr = fbInfo;
	memcpy(m_rtHandles.getBegin(), colorRenderTargetHandles.getBegin(), colorRenderTargetHandles.getSizeInBytes());
	m_rtHandles[kMaxColorRenderTargets] = depthStencilRenderTargetHandle;
	m_rtHandles[kMaxColorRenderTargets + 1] = shadingRateRenderTargetHandle;
	m_fbRenderArea = {minx, miny, maxx, maxy};
}

inline RenderGraphDescription::~RenderGraphDescription()
{
	for(RenderPassDescriptionBase* pass : m_passes)
	{
		deleteInstance(*m_pool, pass);
	}
	m_passes.destroy(*m_pool);
	m_renderTargets.destroy(*m_pool);
	m_buffers.destroy(*m_pool);
	m_as.destroy(*m_pool);
}

inline GraphicsRenderPassDescription& RenderGraphDescription::newGraphicsRenderPass(CString name)
{
	GraphicsRenderPassDescription* pass = newInstance<GraphicsRenderPassDescription>(*m_pool, this);
	pass->m_pool = m_pool;
	pass->setName(name);
	m_passes.emplaceBack(*m_pool, pass);
	return *pass;
}

inline ComputeRenderPassDescription& RenderGraphDescription::newComputeRenderPass(CString name)
{
	ComputeRenderPassDescription* pass = newInstance<ComputeRenderPassDescription>(*m_pool, this);
	pass->m_pool = m_pool;
	pass->setName(name);
	m_passes.emplaceBack(*m_pool, pass);
	return *pass;
}

inline RenderTargetHandle RenderGraphDescription::importRenderTarget(TexturePtr tex, TextureUsageBit usage)
{
	for([[maybe_unused]] const RT& rt : m_renderTargets)
	{
		ANKI_ASSERT(rt.m_importedTex != tex && "Already imported");
	}

	RT& rt = *m_renderTargets.emplaceBack(*m_pool);
	rt.m_importedTex = tex;
	rt.m_importedLastKnownUsage = usage;
	rt.m_usageDerivedByDeps = TextureUsageBit::kNone;
	rt.setName(tex->getName());

	RenderTargetHandle out;
	out.m_idx = m_renderTargets.getSize() - 1;
	return out;
}

inline RenderTargetHandle RenderGraphDescription::importRenderTarget(TexturePtr tex)
{
	RenderTargetHandle out = importRenderTarget(tex, TextureUsageBit::kNone);
	m_renderTargets.getBack().m_importedAndUndefinedUsage = true;
	return out;
}

inline RenderTargetHandle RenderGraphDescription::newRenderTarget(const RenderTargetDescription& initInf)
{
	ANKI_ASSERT(initInf.m_hash && "Forgot to call RenderTargetDescription::bake");
	ANKI_ASSERT(initInf.m_usage == TextureUsageBit::kNone
				&& "Don't need to supply the usage. Render grap will find it");
	RT& rt = *m_renderTargets.emplaceBack(*m_pool);
	rt.m_initInfo = initInf;
	rt.m_hash = initInf.m_hash;
	rt.m_importedLastKnownUsage = TextureUsageBit::kNone;
	rt.m_usageDerivedByDeps = TextureUsageBit::kNone;
	rt.setName(initInf.getName());

	RenderTargetHandle out;
	out.m_idx = m_renderTargets.getSize() - 1;
	return out;
}

inline BufferHandle RenderGraphDescription::importBuffer(BufferPtr buff, BufferUsageBit usage, PtrSize offset,
														 PtrSize range)
{
	// Checks
	if(range == kMaxPtrSize)
	{
		ANKI_ASSERT(offset < buff->getSize());
	}
	else
	{
		ANKI_ASSERT((offset + range) <= buff->getSize());
	}

	for([[maybe_unused]] const Buffer& bb : m_buffers)
	{
		ANKI_ASSERT((bb.m_importedBuff != buff || !bufferRangeOverlaps(bb.m_offset, bb.m_range, offset, range))
					&& "Range already imported");
	}

	Buffer& b = *m_buffers.emplaceBack(*m_pool);
	b.setName(buff->getName());
	b.m_usage = usage;
	b.m_importedBuff = std::move(buff);
	b.m_offset = offset;
	b.m_range = range;

	BufferHandle out;
	out.m_idx = m_buffers.getSize() - 1;
	return out;
}

inline AccelerationStructureHandle
RenderGraphDescription::importAccelerationStructure(AccelerationStructurePtr as, AccelerationStructureUsageBit usage)
{
	for([[maybe_unused]] const AS& a : m_as)
	{
		ANKI_ASSERT(a.m_importedAs != as && "Already imported");
	}

	AS& a = *m_as.emplaceBack(*m_pool);
	a.setName(as->getName());
	a.m_importedAs = std::move(as);
	a.m_usage = usage;

	AccelerationStructureHandle handle;
	handle.m_idx = m_as.getSize() - 1;
	return handle;
}

} // end namespace anki
